/*
 * Copyright (C) 2008, 2009 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "StorageAreaSync.h"

#if ENABLE(DOM_STORAGE)

#include "CString.h"
#include "EventNames.h"
#include "HTMLElement.h"
#include "SQLiteStatement.h"
#include "StorageAreaImpl.h"
#include "StorageSyncManager.h"
#include "SuddenTermination.h"

namespace WebCore {

// If the StorageArea undergoes rapid changes, don't sync each change to disk.
// Instead, queue up a batch of items to sync and actually do the sync at the following interval.
static const double StorageSyncInterval = 1.0;

PassRefPtr<StorageAreaSync> StorageAreaSync::create(PassRefPtr<StorageSyncManager> storageSyncManager, PassRefPtr<StorageAreaImpl> storageArea)
{
    return adoptRef(new StorageAreaSync(storageSyncManager, storageArea));
}

StorageAreaSync::StorageAreaSync(PassRefPtr<StorageSyncManager> storageSyncManager, PassRefPtr<StorageAreaImpl> storageArea)
    : m_syncTimer(this, &StorageAreaSync::syncTimerFired)
    , m_itemsCleared(false)
    , m_finalSyncScheduled(false)
    , m_storageArea(storageArea)
    , m_syncManager(storageSyncManager)
    , m_clearItemsWhileSyncing(false)
    , m_syncScheduled(false)
    , m_importComplete(false)
{
    ASSERT(m_storageArea);
    ASSERT(m_syncManager);

    // FIXME: If it can't import, then the default WebKit behavior should be that of private browsing,
    // not silently ignoring it.  https://bugs.webkit.org/show_bug.cgi?id=25894
    if (!m_syncManager->scheduleImport(this))
        m_importComplete = true;
}

StorageAreaSync::~StorageAreaSync()
{
    ASSERT(!m_syncTimer.isActive());
}

void StorageAreaSync::scheduleFinalSync()
{
    ASSERT(isMainThread());
    // FIXME: We do this to avoid races, but it'd be better to make things safe without blocking.
    blockUntilImportComplete();
    
    if (m_syncTimer.isActive())
        m_syncTimer.stop();
    else {
        // The following is balanced by the call to enableSuddenTermination in the
        // syncTimerFired function.
        disableSuddenTermination();
    }
    // FIXME: This is synchronous.  We should do it on the background process, but
    // we should do it safely.
    syncTimerFired(&m_syncTimer);
    m_finalSyncScheduled = true;
}

void StorageAreaSync::scheduleItemForSync(const String& key, const String& value)
{
    ASSERT(isMainThread());
    ASSERT(!m_finalSyncScheduled);

    m_changedItems.set(key, value);
    if (!m_syncTimer.isActive()) {
        m_syncTimer.startOneShot(StorageSyncInterval);

        // The following is balanced by the call to enableSuddenTermination in the
        // syncTimerFired function.
        disableSuddenTermination();
    }
}

void StorageAreaSync::scheduleClear()
{
    ASSERT(isMainThread());
    ASSERT(!m_finalSyncScheduled);

    m_changedItems.clear();
    m_itemsCleared = true;
    if (!m_syncTimer.isActive()) {
        m_syncTimer.startOneShot(StorageSyncInterval);

        // The following is balanced by the call to enableSuddenTermination in the
        // syncTimerFired function.
        disableSuddenTermination();
    }
}

void StorageAreaSync::syncTimerFired(Timer<StorageAreaSync>*)
{
    ASSERT(isMainThread());

    HashMap<String, String>::iterator it = m_changedItems.begin();
    HashMap<String, String>::iterator end = m_changedItems.end();
    
    {
        MutexLocker locker(m_syncLock);

        if (m_itemsCleared) {
            m_itemsPendingSync.clear();
            m_clearItemsWhileSyncing = true;
            m_itemsCleared = false;
        }

        for (; it != end; ++it)
            m_itemsPendingSync.set(it->first.copy(), it->second.copy());

        if (!m_syncScheduled) {
            m_syncScheduled = true;

            // The following is balanced by the call to enableSuddenTermination in the
            // performSync function.
            disableSuddenTermination();

            m_syncManager->scheduleSync(this);
        }
    }

    // The following is balanced by the calls to disableSuddenTermination in the
    // scheduleItemForSync, scheduleClear, and scheduleFinalSync functions.
    enableSuddenTermination();

    m_changedItems.clear();
}

void StorageAreaSync::performImport()
{
    ASSERT(!isMainThread());
    ASSERT(!m_database.isOpen());

    String databaseFilename = m_syncManager->fullDatabaseFilename(m_storageArea->securityOrigin());

    if (databaseFilename.isEmpty()) {
        LOG_ERROR("Filename for local storage database is empty - cannot open for persistent storage");
        markImported();
        return;
    }

    if (!m_database.open(databaseFilename)) {
        LOG_ERROR("Failed to open database file %s for local storage", databaseFilename.utf8().data());
        markImported();
        return;
    }

    if (!m_database.executeCommand("CREATE TABLE IF NOT EXISTS ItemTable (key TEXT UNIQUE ON CONFLICT REPLACE, value TEXT NOT NULL ON CONFLICT FAIL)")) {
        LOG_ERROR("Failed to create table ItemTable for local storage");
        markImported();
        return;
    }
    
    SQLiteStatement query(m_database, "SELECT key, value FROM ItemTable");
    if (query.prepare() != SQLResultOk) {
        LOG_ERROR("Unable to select items from ItemTable for local storage");
        markImported();
        return;
    }
    
    HashMap<String, String> itemMap;

    int result = query.step();
    while (result == SQLResultRow) {
        itemMap.set(query.getColumnText(0), query.getColumnText(1));
        result = query.step();
    }

    if (result != SQLResultDone) {
        LOG_ERROR("Error reading items from ItemTable for local storage");
        markImported();
        return;
    }

    MutexLocker locker(m_importLock);
    
    HashMap<String, String>::iterator it = itemMap.begin();
    HashMap<String, String>::iterator end = itemMap.end();
    
    for (; it != end; ++it)
        m_storageArea->importItem(it->first, it->second);
    
    // Break the (ref count) cycle.
    m_storageArea = 0;
    m_importComplete = true;
    m_importCondition.signal();
}

void StorageAreaSync::markImported()
{
    ASSERT(!isMainThread());

    MutexLocker locker(m_importLock);
    // Break the (ref count) cycle.
    m_storageArea = 0;
    m_importComplete = true;
    m_importCondition.signal();
}

// FIXME: In the future, we should allow use of StorageAreas while it's importing (when safe to do so).
// Blocking everything until the import is complete is by far the simplest and safest thing to do, but
// there is certainly room for safe optimization: Key/length will never be able to make use of such an
// optimization (since the order of iteration can change as items are being added). Get can return any
// item currently in the map. Get/remove can work whether or not it's in the map, but we'll need a list
// of items the import should not overwrite. Clear can also work, but it'll need to kill the import
// job first.
void StorageAreaSync::blockUntilImportComplete() const
{
    ASSERT(isMainThread());

    // Fast path to avoid locking.
    if (m_importComplete)
        return;

    MutexLocker locker(m_importLock);
    while (!m_importComplete)
        m_importCondition.wait(m_importLock);
    ASSERT(m_importComplete);
    ASSERT(!m_storageArea);
}

void StorageAreaSync::sync(bool clearItems, const HashMap<String, String>& items)
{
    ASSERT(!isMainThread());

    if (!m_database.isOpen())
        return;

    // If the clear flag is set, then we clear all items out before we write any new ones in.
    if (clearItems) {
        SQLiteStatement clear(m_database, "DELETE FROM ItemTable");
        if (clear.prepare() != SQLResultOk) {
            LOG_ERROR("Failed to prepare clear statement - cannot write to local storage database");
            return;
        }
        
        int result = clear.step();
        if (result != SQLResultDone) {
            LOG_ERROR("Failed to clear all items in the local storage database - %i", result);
            return;
        }
    }

    SQLiteStatement insert(m_database, "INSERT INTO ItemTable VALUES (?, ?)");
    if (insert.prepare() != SQLResultOk) {
        LOG_ERROR("Failed to prepare insert statement - cannot write to local storage database");
        return;
    }

    SQLiteStatement remove(m_database, "DELETE FROM ItemTable WHERE key=?");
    if (remove.prepare() != SQLResultOk) {
        LOG_ERROR("Failed to prepare delete statement - cannot write to local storage database");
        return;
    }

    HashMap<String, String>::const_iterator end = items.end();

    for (HashMap<String, String>::const_iterator it = items.begin(); it != end; ++it) {
        // Based on the null-ness of the second argument, decide whether this is an insert or a delete.
        SQLiteStatement& query = it->second.isNull() ? remove : insert;        

        query.bindText(1, it->first);

        // If the second argument is non-null, we're doing an insert, so bind it as the value. 
        if (!it->second.isNull())
            query.bindText(2, it->second);

        int result = query.step();
        if (result != SQLResultDone) {
            LOG_ERROR("Failed to update item in the local storage database - %i", result);
            break;
        }

        query.reset();
    }
}

void StorageAreaSync::performSync()
{
    ASSERT(!isMainThread());

    bool clearItems;
    HashMap<String, String> items;
    {
        MutexLocker locker(m_syncLock);

        ASSERT(m_syncScheduled);

        clearItems = m_clearItemsWhileSyncing;
        m_itemsPendingSync.swap(items);

        m_clearItemsWhileSyncing = false;
        m_syncScheduled = false;
    }

    sync(clearItems, items);

    // The following is balanced by the call to disableSuddenTermination in the
    // syncTimerFired function.
    enableSuddenTermination();
}

} // namespace WebCore

#endif // ENABLE(DOM_STORAGE)

