/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "qwebhistory.h"
#include "qwebhistory_p.h"

#include "PlatformString.h"
#include "Image.h"
#include "KURL.h"
#include "Page.h"
#include "PageGroup.h"

#include <QSharedData>
#include <QDebug>

/*!
  \class QWebHistoryItem
  \since 4.4
  \brief The QWebHistoryItem class represents one item in the history of a QWebPage

  Each QWebHistoryItem instance represents an entry in the history stack of a Web page,
  containing information about the page, its location, and when it was last visited.

  The following table shows the properties of the page held by the history item, and
  the functions used to access them.

  \table
  \header \o Function      \o Description
  \row    \o title()       \o The page title.
  \row    \o url()         \o The location of the page.
  \row    \o originalUrl() \o The URL used to access the page.
  \row    \o lastVisited() \o The date and time of the user's last visit to the page.
  \row    \o icon()        \o The icon associated with the page that was provided by the server.
  \row    \o userData()    \o The user specific data that was stored with the history item.
  \endtable

  \note QWebHistoryItem objects are value based, but \e{explicitly shared}. Changing
  a QWebHistoryItem instance by calling setUserData() will change all copies of that
  instance.

  \sa QWebHistory, QWebPage::history(), QWebHistoryInterface
*/

/*!
  Constructs a history item from \a other. The new item and \a other
  will share their data, and modifying either this item or \a other will
  modify both instances.
*/
QWebHistoryItem::QWebHistoryItem(const QWebHistoryItem &other)
    : d(other.d)
{
}

/*!
  Assigns the \a other history item to this. This item and \a other
  will share their data, and modifying either this item or \a other will
  modify both instances.
*/
QWebHistoryItem &QWebHistoryItem::operator=(const QWebHistoryItem &other)
{
    d = other.d;
    return *this;
}

/*!
  Destroys the history item.
*/
QWebHistoryItem::~QWebHistoryItem()
{
}

/*!
 Returns the original URL associated with the history item.

 \sa url()
*/
QUrl QWebHistoryItem::originalUrl() const
{
    if (d->item)
        return QUrl(d->item->originalURL().string());
    return QUrl();
}


/*!
 Returns the URL associated with the history item.

 \sa originalUrl(), title(), lastVisited()
*/
QUrl QWebHistoryItem::url() const
{
    if (d->item)
        return QUrl(d->item->url().string());
    return QUrl();
}


/*!
 Returns the title of the page associated with the history item.

 \sa icon(), url(), lastVisited()
*/
QString QWebHistoryItem::title() const
{
    if (d->item)
        return d->item->title();
    return QString();
}


/*!
 Returns the date and time that the page associated with the item was last visited.

 \sa title(), icon(), url()
*/
QDateTime QWebHistoryItem::lastVisited() const
{
    //FIXME : this will be wrong unless we correctly set lastVisitedTime ourselves
    if (d->item)
        return QDateTime::fromTime_t((uint)d->item->lastVisitedTime());
    return QDateTime();
}


/*!
 Returns the icon associated with the history item.

 \sa title(), url(), lastVisited()
*/
QIcon QWebHistoryItem::icon() const
{
    if (d->item)
        return *d->item->icon()->nativeImageForCurrentFrame();
    return QIcon();
}

/*!
  \since 4.5
  Returns the user specific data that was stored with the history item.

  \sa setUserData()
*/
QVariant QWebHistoryItem::userData() const
{
    if (d->item)
        return d->item->userData();
    return QVariant();
}

/*!
  \since 4.5

 Stores user specific data \a userData with the history item.
 
 \note All copies of this item will be modified.

 \sa userData()
*/
void QWebHistoryItem::setUserData(const QVariant& userData)
{
    if (d->item)
        d->item->setUserData(userData);
}

/*!*
  \internal
*/
QWebHistoryItem::QWebHistoryItem(QWebHistoryItemPrivate *priv)
{
    d = priv;
}

/*!
    \since 4.5
    Returns whether this is a valid history item.
*/
bool QWebHistoryItem::isValid() const
{
    return d->item;
}

/*!
  \class QWebHistory
  \since 4.4
  \brief The QWebHistory class represents the history of a QWebPage

  Each QWebPage instance contains a history of visited pages that can be accessed
  by QWebPage::history(). QWebHistory represents this history and makes it possible
  to navigate it.

  The history uses the concept of a \e{current item}, dividing the pages visited
  into those that can be visited by navigating \e back and \e forward using the
  back() and forward() functions. The current item can be obtained by calling
  currentItem(), and an arbitrary item in the history can be made the current
  item by passing it to goToItem().

  A list of items describing the pages that can be visited by going back can be
  obtained by calling the backItems() function; similarly, items describing the
  pages ahead of the current page can be obtained with the forwardItems() function.
  The total list of items is obtained with the items() function.

  Just as with containers, functions are available to examine the history in terms
  of a list. Arbitrary items in the history can be obtained with itemAt(), the total
  number of items is given by count(), and the history can be cleared with the
  clear() function.

  QWebHistory's state can be saved with saveState() and loaded with restoreState().

  \sa QWebHistoryItem, QWebHistoryInterface, QWebPage
*/


QWebHistory::QWebHistory()
    : d(0)
{
}

QWebHistory::~QWebHistory()
{
    delete d;
}

/*!
  Clears the history.

  \sa count(), items()
*/
void QWebHistory::clear()
{
    RefPtr<WebCore::HistoryItem> current = d->lst->currentItem();
    int capacity = d->lst->capacity();
    d->lst->setCapacity(0);

    WebCore::Page* page = d->lst->page();
    if (page && page->groupPtr())
        page->groupPtr()->removeVisitedLinks();

    d->lst->setCapacity(capacity);
    d->lst->addItem(current.get());
    d->lst->goToItem(current.get());
}

/*!
  Returns a list of all items currently in the history.

  \sa count(), clear()
*/
QList<QWebHistoryItem> QWebHistory::items() const
{
    const WebCore::HistoryItemVector &items = d->lst->entries();

    QList<QWebHistoryItem> ret;
    for (int i = 0; i < items.size(); ++i) {
        QWebHistoryItemPrivate *priv = new QWebHistoryItemPrivate(items[i].get());
        ret.append(QWebHistoryItem(priv));
    }
    return ret;
}

/*!
  Returns the list of items in the backwards history list.
  At most \a maxItems entries are returned.

  \sa forwardItems()
*/
QList<QWebHistoryItem> QWebHistory::backItems(int maxItems) const
{
    WebCore::HistoryItemVector items(maxItems);
    d->lst->backListWithLimit(maxItems, items);

    QList<QWebHistoryItem> ret;
    for (int i = 0; i < items.size(); ++i) {
        QWebHistoryItemPrivate *priv = new QWebHistoryItemPrivate(items[i].get());
        ret.append(QWebHistoryItem(priv));
    }
    return ret;
}

/*!
  Returns the list of items in the forward history list.
  At most \a maxItems entries are returned.

  \sa backItems()
*/
QList<QWebHistoryItem> QWebHistory::forwardItems(int maxItems) const
{
    WebCore::HistoryItemVector items(maxItems);
    d->lst->forwardListWithLimit(maxItems, items);

    QList<QWebHistoryItem> ret;
    for (int i = 0; i < items.size(); ++i) {
        QWebHistoryItemPrivate *priv = new QWebHistoryItemPrivate(items[i].get());
        ret.append(QWebHistoryItem(priv));
    }
    return ret;
}

/*!
  Returns true if there is an item preceding the current item in the history;
  otherwise returns false.

  \sa canGoForward()
*/
bool QWebHistory::canGoBack() const
{
    return d->lst->backListCount() > 0;
}

/*!
  Returns true if we have an item to go forward to; otherwise returns false.

  \sa canGoBack()
*/
bool QWebHistory::canGoForward() const
{
    return d->lst->forwardListCount() > 0;
}

/*!
  Set the current item to be the previous item in the history and goes to the
  corresponding page; i.e., goes back one history item.

  \sa forward(), goToItem()
*/
void QWebHistory::back()
{
    d->lst->goBack();
    WebCore::Page* page = d->lst->page();
    page->goToItem(currentItem().d->item, WebCore::FrameLoadTypeIndexedBackForward);
}

/*!
  Sets the current item to be the next item in the history and goes to the
  corresponding page; i.e., goes forward one history item.

  \sa back(), goToItem()
*/
void QWebHistory::forward()
{
    d->lst->goForward();
    WebCore::Page* page = d->lst->page();
    page->goToItem(currentItem().d->item, WebCore::FrameLoadTypeIndexedBackForward);
}

/*!
  Sets the current item to be the specified \a item in the history and goes to the page.

  \sa back(), forward()
*/
void QWebHistory::goToItem(const QWebHistoryItem &item)
{
    d->lst->goToItem(item.d->item);
    WebCore::Page* page = d->lst->page();
    page->goToItem(currentItem().d->item, WebCore::FrameLoadTypeIndexedBackForward);
}

/*!
  Returns the item before the current item in the history.
*/
QWebHistoryItem QWebHistory::backItem() const
{
    WebCore::HistoryItem *i = d->lst->backItem();
    QWebHistoryItemPrivate *priv = new QWebHistoryItemPrivate(i);
    return QWebHistoryItem(priv);
}

/*!
  Returns the current item in the history.
*/
QWebHistoryItem QWebHistory::currentItem() const
{
    WebCore::HistoryItem *i = d->lst->currentItem();
    QWebHistoryItemPrivate *priv = new QWebHistoryItemPrivate(i);
    return QWebHistoryItem(priv);
}

/*!
  Returns the item after the current item in the history.
*/
QWebHistoryItem QWebHistory::forwardItem() const
{
    WebCore::HistoryItem *i = d->lst->forwardItem();
    QWebHistoryItemPrivate *priv = new QWebHistoryItemPrivate(i);
    return QWebHistoryItem(priv);
}

/*!
  \since 4.5
  Returns the index of the current item in history.
*/
int QWebHistory::currentItemIndex() const
{
    return d->lst->backListCount();
}

/*!
  Returns the item at index \a i in the history.
*/
QWebHistoryItem QWebHistory::itemAt(int i) const
{
    QWebHistoryItemPrivate *priv;
    if (i < 0 || i >= count())
        priv = new QWebHistoryItemPrivate(0);
    else {
        WebCore::HistoryItem *item = d->lst->entries()[i].get();
        priv = new QWebHistoryItemPrivate(item);
    }
    return QWebHistoryItem(priv);
}

/*!
    Returns the total number of items in the history.
*/
int QWebHistory::count() const
{
    return d->lst->entries().size();
}

/*!
  \since 4.5
  Returns the maximum number of items in the history.

  \sa setMaximumItemCount()
*/
int QWebHistory::maximumItemCount() const
{
    return d->lst->capacity();
}

/*!
  \since 4.5
  Sets the maximum number of items in the history to \a count.

  \sa maximumItemCount()
*/
void QWebHistory::setMaximumItemCount(int count)
{
    d->lst->setCapacity(count);
}

/*!
   \enum QWebHistory::HistoryStateVersion

   This enum describes the versions available for QWebHistory's saveState() function:

   \value HistoryVersion_1 Version 1 (Qt 4.6)
   \value DefaultHistoryVersion The current default version in 1.
*/

/*!
  \since 4.6

  Restores the state of QWebHistory from the given \a buffer. Returns true
  if the history was successfully restored; otherwise returns false.

  \sa saveState()
*/
bool QWebHistory::restoreState(const QByteArray& buffer)
{
    QDataStream stream(buffer);
    int version;
    bool result = false;
    stream >> version;

    switch (version) {
    case HistoryVersion_1: {
        int count;
        int currentIndex;
        stream >> count >> currentIndex;

        clear();
        // only if there are elements
        if (count) {
            // after clear() is new clear HistoryItem (at the end we had to remove it)
            WebCore::HistoryItem *nullItem = d->lst->currentItem();
            for (int i = 0;i < count;i++) {
                WTF::PassRefPtr<WebCore::HistoryItem> item = WebCore::HistoryItem::create();
                item->restoreState(stream, version);
                d->lst->addItem(item);
            }
            d->lst->removeItem(nullItem);
            goToItem(itemAt(currentIndex));
            result = stream.status() == QDataStream::Ok;
        }
        break;
    }
    default: {} // result is false;
    }

    return result;
};

/*!
  \since 4.6
  Saves the state of this QWebHistory into a QByteArray.

  Saves the current state of this QWebHistory. The version number, \a version, is
  stored as part of the data.

  To restore the saved state, pass the return value to restoreState().

  \sa restoreState()
*/
QByteArray QWebHistory::saveState(HistoryStateVersion version) const
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << version;

    switch (version) {
    case HistoryVersion_1: {
        stream << count() << currentItemIndex();

        const WebCore::HistoryItemVector &items = d->lst->entries();
        for (int i = 0; i < items.size(); i++)
            items[i].get()->saveState(stream, version);

        if (stream.status() != QDataStream::Ok)
            buffer = QByteArray();  // make buffer isNull()==true and isEmpty()==true
        break;
    }
    default:
        buffer.clear();

    }

    return buffer;
}

/*!
  \since 4.6
  \fn QDataStream& operator<<(QDataStream& stream, const QWebHistory& history)
  \relates QWebHistory

  Saves the given \a history into the specified \a stream. This is a convenience function
  and is equivalent to calling the saveState() method.

  \sa QWebHistory::saveState()
*/

QDataStream& operator<<(QDataStream& stream, const QWebHistory& history)
{
    return stream << history.saveState();
}

/*!
  \fn QDataStream& operator>>(QDataStream& stream, QWebHistory& history)
  \relates QWebHistory
  \since 4.6

  Loads a QWebHistory from the specified \a stream into the given \a history.
  This is a convenience function and it is equivalent to calling the restoreState()
  method.

  \sa QWebHistory::restoreState()
*/

QDataStream& operator>>(QDataStream& stream, QWebHistory& history)
{
    QByteArray buffer;
    stream >> buffer;
    history.restoreState(buffer);
    return stream;
}


