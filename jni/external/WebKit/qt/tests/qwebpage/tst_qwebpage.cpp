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


#include <QtTest/QtTest>

#include <qwebpage.h>
#include <qwidget.h>
#include <qwebview.h>
#include <qwebframe.h>
#include <qwebhistory.h>
#include <qnetworkrequest.h>
#include <QDebug>
#include <QLineEdit>
#include <QMenu>
#include <qwebsecurityorigin.h>
#include <qwebdatabase.h>
#include <QPushButton>

// Will try to wait for the condition while allowing event processing
#define QTRY_COMPARE(__expr, __expected) \
    do { \
        const int __step = 50; \
        const int __timeout = 5000; \
        if ((__expr) != (__expected)) { \
            QTest::qWait(0); \
        } \
        for (int __i = 0; __i < __timeout && ((__expr) != (__expected)); __i+=__step) { \
            QTest::qWait(__step); \
        } \
        QCOMPARE(__expr, __expected); \
    } while(0)

//TESTED_CLASS=
//TESTED_FILES=

// Task 160192
/**
 * Starts an event loop that runs until the given signal is received.
 Optionally the event loop
 * can return earlier on a timeout.
 *
 * \return \p true if the requested signal was received
 *         \p false on timeout
 */
static bool waitForSignal(QObject* obj, const char* signal, int timeout = 0)
{
    QEventLoop loop;
    QObject::connect(obj, signal, &loop, SLOT(quit()));
    QTimer timer;
    QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
    if (timeout > 0) {
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);
        timer.start(timeout);
    }
    loop.exec();
    return timeoutSpy.isEmpty();
}

class tst_QWebPage : public QObject
{
    Q_OBJECT

public:
    tst_QWebPage();
    virtual ~tst_QWebPage();

public slots:
    void init();
    void cleanup();
    void cleanupFiles();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void acceptNavigationRequest();
    void infiniteLoopJS();
    void loadFinished();
    void acceptNavigationRequestWithNewWindow();
    void userStyleSheet();
    void modified();
    void contextMenuCrash();
    void database();
    void createPlugin();
    void destroyPlugin();
    void createViewlessPlugin();
    void multiplePageGroupsAndLocalStorage();
    void cursorMovements();
    void textSelection();
    void textEditing();
    void backActionUpdate();
    void frameAt();
    void requestCache();
    void protectBindingsRuntimeObjectsFromCollector();

private:


private:
    QWebView* m_view;
    QWebPage* m_page;
};

tst_QWebPage::tst_QWebPage()
{
}

tst_QWebPage::~tst_QWebPage()
{
}

void tst_QWebPage::init()
{
    m_view = new QWebView();
    m_page = m_view->page();
}

void tst_QWebPage::cleanup()
{
    delete m_view;
}

void tst_QWebPage::cleanupFiles()
{
    QFile::remove("Databases.db");
    QDir::current().rmdir("http_www.myexample.com_0");
    QFile::remove("http_www.myexample.com_0.localstorage");
}

void tst_QWebPage::initTestCase()
{
    cleanupFiles(); // In case there are old files from previous runs
}

void tst_QWebPage::cleanupTestCase()
{
    cleanupFiles(); // Be nice
}

class NavigationRequestOverride : public QWebPage
{
public:
    NavigationRequestOverride(QWebView* parent, bool initialValue) : QWebPage(parent), m_acceptNavigationRequest(initialValue) {}

    bool m_acceptNavigationRequest;
protected:
    virtual bool acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest &request, QWebPage::NavigationType type) {
        Q_UNUSED(frame);
        Q_UNUSED(request);
        Q_UNUSED(type);

        return m_acceptNavigationRequest;
    }
};

void tst_QWebPage::acceptNavigationRequest()
{
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));

    NavigationRequestOverride* newPage = new NavigationRequestOverride(m_view, false);
    m_view->setPage(newPage);

    m_view->setHtml(QString("<html><body><form name='tstform' action='data:text/html,foo'method='get'>"
                            "<input type='text'><input type='submit'></form></body></html>"), QUrl());
    QTRY_COMPARE(loadSpy.count(), 1);

    m_view->page()->mainFrame()->evaluateJavaScript("tstform.submit();");

    newPage->m_acceptNavigationRequest = true;
    m_view->page()->mainFrame()->evaluateJavaScript("tstform.submit();");
    QTRY_COMPARE(loadSpy.count(), 2);

    QCOMPARE(m_view->page()->mainFrame()->toPlainText(), QString("foo?"));

    // Restore default page
    m_view->setPage(0);
}

class JSTestPage : public QWebPage
{
Q_OBJECT
public:
    JSTestPage(QObject* parent = 0)
    : QWebPage(parent) {}

public slots:
    bool shouldInterruptJavaScript() {
        return true; 
    }
};

void tst_QWebPage::infiniteLoopJS()
{
    JSTestPage* newPage = new JSTestPage(m_view);
    m_view->setPage(newPage);
    m_view->setHtml(QString("<html><bodytest</body></html>"), QUrl());
    m_view->page()->mainFrame()->evaluateJavaScript("var run = true;var a = 1;while(run){a++;}");
}

void tst_QWebPage::loadFinished()
{
    QSignalSpy spyLoadStarted(m_view, SIGNAL(loadStarted()));
    QSignalSpy spyLoadFinished(m_view, SIGNAL(loadFinished(bool)));

    m_view->setHtml(QString("data:text/html,<frameset cols=\"25%,75%\"><frame src=\"data:text/html,"
                            "<head><meta http-equiv='refresh' content='1'></head>foo \">"
                            "<frame src=\"data:text/html,bar\"></frameset>"), QUrl());
    QTRY_COMPARE(spyLoadFinished.count(), 1);

    QTest::qWait(3000);

    QVERIFY(spyLoadStarted.count() > 1);
    QVERIFY(spyLoadFinished.count() > 1);

    spyLoadFinished.clear();

    m_view->setHtml(QString("data:text/html,<frameset cols=\"25%,75%\"><frame src=\"data:text/html,"
                            "foo \"><frame src=\"data:text/html,bar\"></frameset>"), QUrl());
    QTRY_COMPARE(spyLoadFinished.count(), 1);
    QCOMPARE(spyLoadFinished.count(), 1);
}

class TestPage : public QWebPage
{
public:
    TestPage(QObject* parent = 0) : QWebPage(parent) {}

    struct Navigation {
        QPointer<QWebFrame> frame;
        QNetworkRequest request;
        NavigationType type;
    };

    QList<Navigation> navigations;
    QList<QWebPage*> createdWindows;

    virtual bool acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest &request, NavigationType type) {
        Navigation n;
        n.frame = frame;
        n.request = request;
        n.type = type;
        navigations.append(n);
        return true;
    }

    virtual QWebPage* createWindow(WebWindowType) {
        QWebPage* page = new TestPage(this);
        createdWindows.append(page);
        return page;
    }
};

void tst_QWebPage::acceptNavigationRequestWithNewWindow()
{
    TestPage* page = new TestPage(m_view);
    page->settings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
    m_page = page;
    m_view->setPage(m_page);

    m_view->setUrl(QString("data:text/html,<a href=\"data:text/html,Reached\" target=\"_blank\">Click me</a>"));
    QVERIFY(::waitForSignal(m_view, SIGNAL(loadFinished(bool))));

    QFocusEvent fe(QEvent::FocusIn);
    m_page->event(&fe);

    QVERIFY(m_page->focusNextPrevChild(/*next*/ true));

    QKeyEvent keyEnter(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    m_page->event(&keyEnter);

    QCOMPARE(page->navigations.count(), 2);

    TestPage::Navigation n = page->navigations.at(1);
    QVERIFY(n.frame.isNull());
    QCOMPARE(n.request.url().toString(), QString("data:text/html,Reached"));
    QVERIFY(n.type == QWebPage::NavigationTypeLinkClicked);

    QCOMPARE(page->createdWindows.count(), 1);
}

class TestNetworkManager : public QNetworkAccessManager
{
public:
    TestNetworkManager(QObject* parent) : QNetworkAccessManager(parent) {}

    QList<QUrl> requestedUrls;

protected:
    virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest &request, QIODevice* outgoingData) {
        requestedUrls.append(request.url());
        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    }
};

void tst_QWebPage::userStyleSheet()
{
    TestNetworkManager* networkManager = new TestNetworkManager(m_page);
    m_page->setNetworkAccessManager(networkManager);
    networkManager->requestedUrls.clear();

    m_page->settings()->setUserStyleSheetUrl(QUrl("data:text/css,p { background-image: url('http://does.not/exist.png');}"));
    m_view->setHtml("<p>hello world</p>");
    QVERIFY(::waitForSignal(m_view, SIGNAL(loadFinished(bool))));

    QVERIFY(networkManager->requestedUrls.count() >= 2);
    QCOMPARE(networkManager->requestedUrls.at(0), QUrl("data:text/css,p { background-image: url('http://does.not/exist.png');}"));
    QCOMPARE(networkManager->requestedUrls.at(1), QUrl("http://does.not/exist.png"));
}

void tst_QWebPage::modified()
{
    m_page->mainFrame()->setUrl(QUrl("data:text/html,<body>blub"));
    QVERIFY(::waitForSignal(m_view, SIGNAL(loadFinished(bool))));

    m_page->mainFrame()->setUrl(QUrl("data:text/html,<body id=foo contenteditable>blah"));
    QVERIFY(::waitForSignal(m_view, SIGNAL(loadFinished(bool))));

    QVERIFY(!m_page->isModified());

//    m_page->mainFrame()->evaluateJavaScript("alert(document.getElementById('foo'))");
    m_page->mainFrame()->evaluateJavaScript("document.getElementById('foo').focus()");
    m_page->mainFrame()->evaluateJavaScript("document.execCommand('InsertText', true, 'Test');");

    QVERIFY(m_page->isModified());

    m_page->mainFrame()->evaluateJavaScript("document.execCommand('Undo', true);");

    QVERIFY(!m_page->isModified());

    m_page->mainFrame()->evaluateJavaScript("document.execCommand('Redo', true);");

    QVERIFY(m_page->isModified());

    QVERIFY(m_page->history()->canGoBack());
    QVERIFY(!m_page->history()->canGoForward());
    QCOMPARE(m_page->history()->count(), 2);
    QVERIFY(m_page->history()->backItem().isValid());
    QVERIFY(!m_page->history()->forwardItem().isValid());

    m_page->history()->back();
    QVERIFY(::waitForSignal(m_view, SIGNAL(loadFinished(bool))));

    QVERIFY(!m_page->history()->canGoBack());
    QVERIFY(m_page->history()->canGoForward());

    QVERIFY(!m_page->isModified());

    QVERIFY(m_page->history()->currentItemIndex() == 0);

    m_page->history()->setMaximumItemCount(3);
    QVERIFY(m_page->history()->maximumItemCount() == 3);

    QVariant variant("string test");
    m_page->history()->currentItem().setUserData(variant);
    QVERIFY(m_page->history()->currentItem().userData().toString() == "string test");

    m_page->mainFrame()->setUrl(QUrl("data:text/html,<body>This is second page"));
    m_page->mainFrame()->setUrl(QUrl("data:text/html,<body>This is third page"));
    QVERIFY(m_page->history()->count() == 2);
    m_page->mainFrame()->setUrl(QUrl("data:text/html,<body>This is fourth page"));
    QVERIFY(m_page->history()->count() == 2);
    m_page->mainFrame()->setUrl(QUrl("data:text/html,<body>This is fifth page"));
    QVERIFY(::waitForSignal(m_page, SIGNAL(saveFrameStateRequested(QWebFrame*, QWebHistoryItem*))));
}

void tst_QWebPage::contextMenuCrash()
{
    QWebView view;
    view.setHtml("<p>test");
    view.page()->updatePositionDependentActions(QPoint(0, 0));
    QMenu* contextMenu = 0;
    foreach (QObject* child, view.children()) {
        contextMenu = qobject_cast<QMenu*>(child);
        if (contextMenu)
            break;
    }
    QVERIFY(contextMenu);
    delete contextMenu;
}

void tst_QWebPage::database()
{
    QString path = QDir::currentPath();
    m_page->settings()->setOfflineStoragePath(path);
    QVERIFY(m_page->settings()->offlineStoragePath() == path);

    QWebSettings::setOfflineStorageDefaultQuota(1024 * 1024);
    QVERIFY(QWebSettings::offlineStorageDefaultQuota() == 1024 * 1024);

    QString dbFileName = path + "Databases.db";

    if (QFile::exists(dbFileName))
        QFile::remove(dbFileName);

    qRegisterMetaType<QWebFrame*>("QWebFrame*");
    QSignalSpy spy(m_page, SIGNAL(databaseQuotaExceeded(QWebFrame *, QString)));
    m_view->setHtml(QString("<html><head><script>var db; db=openDatabase('testdb', '1.0', 'test database API', 50000); </script></head><body><div></div></body></html>"), QUrl("http://www.myexample.com"));
    QTRY_COMPARE(spy.count(), 1);
    m_page->mainFrame()->evaluateJavaScript("var db2; db2=openDatabase('testdb', '1.0', 'test database API', 50000);");
    QTRY_COMPARE(spy.count(),1);

    m_page->mainFrame()->evaluateJavaScript("localStorage.test='This is a test for local storage';");
    m_view->setHtml(QString("<html><body id='b'>text</body></html>"), QUrl("http://www.myexample.com"));

    QVariant s1 = m_page->mainFrame()->evaluateJavaScript("localStorage.test");
    QCOMPARE(s1.toString(), QString("This is a test for local storage"));

    m_page->mainFrame()->evaluateJavaScript("sessionStorage.test='This is a test for session storage';");
    m_view->setHtml(QString("<html><body id='b'>text</body></html>"), QUrl("http://www.myexample.com"));
    QVariant s2 = m_page->mainFrame()->evaluateJavaScript("sessionStorage.test");
    QCOMPARE(s2.toString(), QString("This is a test for session storage"));

    m_view->setHtml(QString("<html><head></head><body><div></div></body></html>"), QUrl("http://www.myexample.com"));
    m_page->mainFrame()->evaluateJavaScript("var db3; db3=openDatabase('testdb', '1.0', 'test database API', 50000);db3.transaction(function(tx) { tx.executeSql('CREATE TABLE IF NOT EXISTS Test (text TEXT)', []); }, function(tx, result) { }, function(tx, error) { });");
    QTest::qWait(200);

    QWebSecurityOrigin origin = m_page->mainFrame()->securityOrigin();
    QList<QWebDatabase> dbs = origin.databases();
    if (dbs.count() > 0) {
        QString fileName = dbs[0].fileName();
        QVERIFY(QFile::exists(fileName));
        QWebDatabase::removeDatabase(dbs[0]);
        QVERIFY(!QFile::exists(fileName));
    }
    QTest::qWait(1000);
}

class PluginPage : public QWebPage
{
public:
    PluginPage(QObject *parent = 0)
        : QWebPage(parent) {}

    struct CallInfo
    {
        CallInfo(const QString &c, const QUrl &u,
                 const QStringList &pn, const QStringList &pv,
                 QObject *r)
            : classid(c), url(u), paramNames(pn),
              paramValues(pv), returnValue(r)
            {}
        QString classid;
        QUrl url;
        QStringList paramNames;
        QStringList paramValues;
        QObject *returnValue;
    };

    QList<CallInfo> calls;

protected:
    virtual QObject *createPlugin(const QString &classid, const QUrl &url,
                                  const QStringList &paramNames,
                                  const QStringList &paramValues)
    {
        QObject *result = 0;
        if (classid == "pushbutton")
            result = new QPushButton();
        else if (classid == "lineedit")
            result = new QLineEdit();
        if (result)
            result->setObjectName(classid);
        calls.append(CallInfo(classid, url, paramNames, paramValues, result));
        return result;
    }
};

void tst_QWebPage::createPlugin()
{
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));

    PluginPage* newPage = new PluginPage(m_view);
    m_view->setPage(newPage);

    // plugins not enabled by default, so the plugin shouldn't be loaded
    m_view->setHtml(QString("<html><body><object type='application/x-qt-plugin' classid='pushbutton' id='mybutton'/></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 1);
    QCOMPARE(newPage->calls.count(), 0);

    m_view->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    // type has to be application/x-qt-plugin
    m_view->setHtml(QString("<html><body><object type='application/x-foobarbaz' classid='pushbutton' id='mybutton'/></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 2);
    QCOMPARE(newPage->calls.count(), 0);

    m_view->setHtml(QString("<html><body><object type='application/x-qt-plugin' classid='pushbutton' id='mybutton'/></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 3);
    QCOMPARE(newPage->calls.count(), 1);
    {
        PluginPage::CallInfo ci = newPage->calls.takeFirst();
        QCOMPARE(ci.classid, QString::fromLatin1("pushbutton"));
        QCOMPARE(ci.url, QUrl());
        QCOMPARE(ci.paramNames.count(), 3);
        QCOMPARE(ci.paramValues.count(), 3);
        QCOMPARE(ci.paramNames.at(0), QString::fromLatin1("type"));
        QCOMPARE(ci.paramValues.at(0), QString::fromLatin1("application/x-qt-plugin"));
        QCOMPARE(ci.paramNames.at(1), QString::fromLatin1("classid"));
        QCOMPARE(ci.paramValues.at(1), QString::fromLatin1("pushbutton"));
        QCOMPARE(ci.paramNames.at(2), QString::fromLatin1("id"));
        QCOMPARE(ci.paramValues.at(2), QString::fromLatin1("mybutton"));
        QVERIFY(ci.returnValue != 0);
        QVERIFY(ci.returnValue->inherits("QPushButton"));
    }
    // test JS bindings
    QCOMPARE(newPage->mainFrame()->evaluateJavaScript("document.getElementById('mybutton').toString()").toString(),
             QString::fromLatin1("[object HTMLObjectElement]"));
    QCOMPARE(newPage->mainFrame()->evaluateJavaScript("mybutton.toString()").toString(),
             QString::fromLatin1("[object HTMLObjectElement]"));
    QCOMPARE(newPage->mainFrame()->evaluateJavaScript("typeof mybutton.objectName").toString(),
             QString::fromLatin1("string"));
    QCOMPARE(newPage->mainFrame()->evaluateJavaScript("mybutton.objectName").toString(),
             QString::fromLatin1("pushbutton"));
    QCOMPARE(newPage->mainFrame()->evaluateJavaScript("typeof mybutton.clicked").toString(),
             QString::fromLatin1("function"));
    QCOMPARE(newPage->mainFrame()->evaluateJavaScript("mybutton.clicked.toString()").toString(),
             QString::fromLatin1("function clicked() {\n    [native code]\n}"));

    m_view->setHtml(QString("<html><body><table>"
                            "<tr><object type='application/x-qt-plugin' classid='lineedit' id='myedit'/></tr>"
                            "<tr><object type='application/x-qt-plugin' classid='pushbutton' id='mybutton'/></tr>"
                            "</table></body></html>"), QUrl("http://foo.bar.baz"));
    QTRY_COMPARE(loadSpy.count(), 4);
    QCOMPARE(newPage->calls.count(), 2);
    {
        PluginPage::CallInfo ci = newPage->calls.takeFirst();
        QCOMPARE(ci.classid, QString::fromLatin1("lineedit"));
        QCOMPARE(ci.url, QUrl());
        QCOMPARE(ci.paramNames.count(), 3);
        QCOMPARE(ci.paramValues.count(), 3);
        QCOMPARE(ci.paramNames.at(0), QString::fromLatin1("type"));
        QCOMPARE(ci.paramValues.at(0), QString::fromLatin1("application/x-qt-plugin"));
        QCOMPARE(ci.paramNames.at(1), QString::fromLatin1("classid"));
        QCOMPARE(ci.paramValues.at(1), QString::fromLatin1("lineedit"));
        QCOMPARE(ci.paramNames.at(2), QString::fromLatin1("id"));
        QCOMPARE(ci.paramValues.at(2), QString::fromLatin1("myedit"));
        QVERIFY(ci.returnValue != 0);
        QVERIFY(ci.returnValue->inherits("QLineEdit"));
    }
    {
        PluginPage::CallInfo ci = newPage->calls.takeFirst();
        QCOMPARE(ci.classid, QString::fromLatin1("pushbutton"));
        QCOMPARE(ci.url, QUrl());
        QCOMPARE(ci.paramNames.count(), 3);
        QCOMPARE(ci.paramValues.count(), 3);
        QCOMPARE(ci.paramNames.at(0), QString::fromLatin1("type"));
        QCOMPARE(ci.paramValues.at(0), QString::fromLatin1("application/x-qt-plugin"));
        QCOMPARE(ci.paramNames.at(1), QString::fromLatin1("classid"));
        QCOMPARE(ci.paramValues.at(1), QString::fromLatin1("pushbutton"));
        QCOMPARE(ci.paramNames.at(2), QString::fromLatin1("id"));
        QCOMPARE(ci.paramValues.at(2), QString::fromLatin1("mybutton"));
        QVERIFY(ci.returnValue != 0);
        QVERIFY(ci.returnValue->inherits("QPushButton"));
    }

    m_view->settings()->setAttribute(QWebSettings::PluginsEnabled, false);

    m_view->setHtml(QString("<html><body><object type='application/x-qt-plugin' classid='pushbutton' id='mybutton'/></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 5);
    QCOMPARE(newPage->calls.count(), 0);
}

class PluginTrackedPage : public QWebPage
{
public:

    int count;
    QPointer<QWidget> widget;

    PluginTrackedPage(QWidget *parent = 0) : QWebPage(parent), count(0) {
       settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    }

    virtual QObject* createPlugin(const QString&, const QUrl&, const QStringList&, const QStringList&) {
       count++;
       QWidget *w = new QWidget;
       widget = w;
       return w;
    }
};

void tst_QWebPage::destroyPlugin()
{
    PluginTrackedPage* page = new PluginTrackedPage(m_view);
    m_view->setPage(page);

    // we create the plugin, so the widget should be constructed
    QString content("<html><body><object type=\"application/x-qt-plugin\" classid=\"QProgressBar\"></object></body></html>");
    m_view->setHtml(content);
    QVERIFY(page->widget != 0);
    QCOMPARE(page->count, 1);

    // navigate away, the plugin widget should be destructed
    m_view->setHtml("<html><body>Hi</body></html>");
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(page->widget == 0);
}

void tst_QWebPage::createViewlessPlugin()
{
    PluginTrackedPage* page = new PluginTrackedPage;
    QString content("<html><body><object type=\"application/x-qt-plugin\" classid=\"QProgressBar\"></object></body></html>");
    page->mainFrame()->setHtml(content);
    QCOMPARE(page->count, 1);
    QVERIFY(page->widget != 0);
    delete page;
}

// import private API
void QWEBKIT_EXPORT qt_webpage_setGroupName(QWebPage* page, const QString& groupName);
QString QWEBKIT_EXPORT qt_webpage_groupName(QWebPage* page);
void QWEBKIT_EXPORT qt_websettings_setLocalStorageDatabasePath(QWebSettings* settings, const QString& path);

void tst_QWebPage::multiplePageGroupsAndLocalStorage()
{
    QDir dir(QDir::currentPath());
    dir.mkdir("path1");
    dir.mkdir("path2");

    QWebView view1;
    QWebView view2;

    qt_websettings_setLocalStorageDatabasePath(view1.page()->settings(), QDir::toNativeSeparators(QDir::currentPath() + "/path1"));
    qt_webpage_setGroupName(view1.page(), "group1");
    qt_websettings_setLocalStorageDatabasePath(view2.page()->settings(), QDir::toNativeSeparators(QDir::currentPath() + "/path2"));
    qt_webpage_setGroupName(view2.page(), "group2");
    QCOMPARE(qt_webpage_groupName(view1.page()), QString("group1"));
    QCOMPARE(qt_webpage_groupName(view2.page()), QString("group2"));


    view1.setHtml(QString("<html><body> </body></html>"), QUrl("http://www.myexample.com"));
    view2.setHtml(QString("<html><body> </body></html>"), QUrl("http://www.myexample.com"));

    view1.page()->mainFrame()->evaluateJavaScript("localStorage.test='value1';");
    view2.page()->mainFrame()->evaluateJavaScript("localStorage.test='value2';");

    view1.setHtml(QString("<html><body> </body></html>"), QUrl("http://www.myexample.com"));
    view2.setHtml(QString("<html><body> </body></html>"), QUrl("http://www.myexample.com"));

    QVariant s1 = view1.page()->mainFrame()->evaluateJavaScript("localStorage.test");
    QCOMPARE(s1.toString(), QString("value1"));

    QVariant s2 = view2.page()->mainFrame()->evaluateJavaScript("localStorage.test");
    QCOMPARE(s2.toString(), QString("value2"));

    QTest::qWait(1000);

    QFile::remove(QDir::toNativeSeparators(QDir::currentPath() + "/path1/http_www.myexample.com_0.localstorage"));
    QFile::remove(QDir::toNativeSeparators(QDir::currentPath() + "/path2/http_www.myexample.com_0.localstorage"));
    dir.rmdir(QDir::toNativeSeparators("./path1"));
    dir.rmdir(QDir::toNativeSeparators("./path2"));
}

class CursorTrackedPage : public QWebPage
{
public:

    CursorTrackedPage(QWidget *parent = 0): QWebPage(parent) {
        setViewportSize(QSize(1024, 768)); // big space
    }

    QString selectedText() {
        return mainFrame()->evaluateJavaScript("window.getSelection().toString()").toString();
    }

    int selectionStartOffset() {
        return mainFrame()->evaluateJavaScript("window.getSelection().getRangeAt(0).startOffset").toInt();
    }

    int selectionEndOffset() {
        return mainFrame()->evaluateJavaScript("window.getSelection().getRangeAt(0).endOffset").toInt();
    }

    // true if start offset == end offset, i.e. no selected text
    int isSelectionCollapsed() {
        return mainFrame()->evaluateJavaScript("window.getSelection().getRangeAt(0).collapsed").toBool();
    }
};

void tst_QWebPage::cursorMovements()
{
    CursorTrackedPage* page = new CursorTrackedPage;
    QString content("<html><body<p id=one>The quick brown fox</p><p id=two>jumps over the lazy dog</p><p>May the source<br/>be with you!</p></body></html>");
    page->mainFrame()->setHtml(content);

    // this will select the first paragraph
    QString script = "var range = document.createRange(); " \
        "var node = document.getElementById(\"one\"); " \
        "range.selectNode(node); " \
        "getSelection().addRange(range);";
    page->mainFrame()->evaluateJavaScript(script);
    QCOMPARE(page->selectedText().trimmed(), QString::fromLatin1("The quick brown fox"));

    // these actions must exist
    QVERIFY(page->action(QWebPage::MoveToNextChar) != 0);
    QVERIFY(page->action(QWebPage::MoveToPreviousChar) != 0);
    QVERIFY(page->action(QWebPage::MoveToNextWord) != 0);
    QVERIFY(page->action(QWebPage::MoveToPreviousWord) != 0);
    QVERIFY(page->action(QWebPage::MoveToNextLine) != 0);
    QVERIFY(page->action(QWebPage::MoveToPreviousLine) != 0);
    QVERIFY(page->action(QWebPage::MoveToStartOfLine) != 0);
    QVERIFY(page->action(QWebPage::MoveToEndOfLine) != 0);
    QVERIFY(page->action(QWebPage::MoveToStartOfBlock) != 0);
    QVERIFY(page->action(QWebPage::MoveToEndOfBlock) != 0);
    QVERIFY(page->action(QWebPage::MoveToStartOfDocument) != 0);
    QVERIFY(page->action(QWebPage::MoveToEndOfDocument) != 0);

    // right now they are disabled because contentEditable is false
    QCOMPARE(page->action(QWebPage::MoveToNextChar)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToPreviousChar)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToNextWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToPreviousWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToNextLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToPreviousLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToStartOfLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToEndOfLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToStartOfBlock)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToEndOfBlock)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToStartOfDocument)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::MoveToEndOfDocument)->isEnabled(), false);

    // make it editable before navigating the cursor
    page->setContentEditable(true);

    // here the actions are enabled after contentEditable is true
    QCOMPARE(page->action(QWebPage::MoveToNextChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToPreviousChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToNextWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToPreviousWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToNextLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToPreviousLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToStartOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToEndOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToStartOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToEndOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToStartOfDocument)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::MoveToEndOfDocument)->isEnabled(), true);

    // cursor will be before the word "jump"
    page->triggerAction(QWebPage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // cursor will be between 'j' and 'u' in the word "jump"
    page->triggerAction(QWebPage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 1);

    // cursor will be between 'u' and 'm' in the word "jump"
    page->triggerAction(QWebPage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 2);

    // cursor will be after the word "jump"
    page->triggerAction(QWebPage::MoveToNextWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 5);

    // cursor will be after the word "lazy"
    page->triggerAction(QWebPage::MoveToNextWord);
    page->triggerAction(QWebPage::MoveToNextWord);
    page->triggerAction(QWebPage::MoveToNextWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 19);

    // cursor will be between 'z' and 'y' in "lazy"
    page->triggerAction(QWebPage::MoveToPreviousChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 18);

    // cursor will be between 'a' and 'z' in "lazy"
    page->triggerAction(QWebPage::MoveToPreviousChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 17);

    // cursor will be before the word "lazy"
    page->triggerAction(QWebPage::MoveToPreviousWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 15);

    // cursor will be before the word "quick"
    page->triggerAction(QWebPage::MoveToPreviousWord);
    page->triggerAction(QWebPage::MoveToPreviousWord);
    page->triggerAction(QWebPage::MoveToPreviousWord);
    page->triggerAction(QWebPage::MoveToPreviousWord);
    page->triggerAction(QWebPage::MoveToPreviousWord);
    page->triggerAction(QWebPage::MoveToPreviousWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 4);

    // cursor will be between 'p' and 's' in the word "jumps"
    page->triggerAction(QWebPage::MoveToNextWord);
    page->triggerAction(QWebPage::MoveToNextWord);
    page->triggerAction(QWebPage::MoveToNextWord);
    page->triggerAction(QWebPage::MoveToNextChar);
    page->triggerAction(QWebPage::MoveToNextChar);
    page->triggerAction(QWebPage::MoveToNextChar);
    page->triggerAction(QWebPage::MoveToNextChar);
    page->triggerAction(QWebPage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 4);

    // cursor will be before the word "jumps"
    page->triggerAction(QWebPage::MoveToStartOfLine);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // cursor will be after the word "dog"
    page->triggerAction(QWebPage::MoveToEndOfLine);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 23);

    // cursor will be between 'w' and 'n' in "brown"
    page->triggerAction(QWebPage::MoveToStartOfLine);
    page->triggerAction(QWebPage::MoveToPreviousWord);
    page->triggerAction(QWebPage::MoveToPreviousWord);
    page->triggerAction(QWebPage::MoveToNextChar);
    page->triggerAction(QWebPage::MoveToNextChar);
    page->triggerAction(QWebPage::MoveToNextChar);
    page->triggerAction(QWebPage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 14);

    // cursor will be after the word "fox"
    page->triggerAction(QWebPage::MoveToEndOfLine);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 19);

    // cursor will be before the word "The"
    page->triggerAction(QWebPage::MoveToStartOfDocument);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // cursor will be after the word "you!"
    page->triggerAction(QWebPage::MoveToEndOfDocument);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 12);

    // cursor will be before the word "be"
    page->triggerAction(QWebPage::MoveToStartOfBlock);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // cursor will be after the word "you!"
    page->triggerAction(QWebPage::MoveToEndOfBlock);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 12);

    // try to move before the document start
    page->triggerAction(QWebPage::MoveToStartOfDocument);
    page->triggerAction(QWebPage::MoveToPreviousChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);
    page->triggerAction(QWebPage::MoveToStartOfDocument);
    page->triggerAction(QWebPage::MoveToPreviousWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // try to move past the document end
    page->triggerAction(QWebPage::MoveToEndOfDocument);
    page->triggerAction(QWebPage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 12);
    page->triggerAction(QWebPage::MoveToEndOfDocument);
    page->triggerAction(QWebPage::MoveToNextWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 12);

    delete page;
}

void tst_QWebPage::textSelection()
{
    CursorTrackedPage* page = new CursorTrackedPage;
    QString content("<html><body<p id=one>The quick brown fox</p>" \
        "<p id=two>jumps over the lazy dog</p>" \
        "<p>May the source<br/>be with you!</p></body></html>");
    page->mainFrame()->setHtml(content);

    // these actions must exist
    QVERIFY(page->action(QWebPage::SelectAll) != 0);
    QVERIFY(page->action(QWebPage::SelectNextChar) != 0);
    QVERIFY(page->action(QWebPage::SelectPreviousChar) != 0);
    QVERIFY(page->action(QWebPage::SelectNextWord) != 0);
    QVERIFY(page->action(QWebPage::SelectPreviousWord) != 0);
    QVERIFY(page->action(QWebPage::SelectNextLine) != 0);
    QVERIFY(page->action(QWebPage::SelectPreviousLine) != 0);
    QVERIFY(page->action(QWebPage::SelectStartOfLine) != 0);
    QVERIFY(page->action(QWebPage::SelectEndOfLine) != 0);
    QVERIFY(page->action(QWebPage::SelectStartOfBlock) != 0);
    QVERIFY(page->action(QWebPage::SelectEndOfBlock) != 0);
    QVERIFY(page->action(QWebPage::SelectStartOfDocument) != 0);
    QVERIFY(page->action(QWebPage::SelectEndOfDocument) != 0);

    // right now they are disabled because contentEditable is false and 
    // there isn't an existing selection to modify
    QCOMPARE(page->action(QWebPage::SelectNextChar)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectPreviousChar)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectNextWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectPreviousWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectNextLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectPreviousLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectStartOfLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectEndOfLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectStartOfBlock)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectEndOfBlock)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectStartOfDocument)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SelectEndOfDocument)->isEnabled(), false);

    // ..but SelectAll is awalys enabled
    QCOMPARE(page->action(QWebPage::SelectAll)->isEnabled(), true);

    // this will select the first paragraph
    QString selectScript = "var range = document.createRange(); " \
        "var node = document.getElementById(\"one\"); " \
        "range.selectNode(node); " \
        "getSelection().addRange(range);";
    page->mainFrame()->evaluateJavaScript(selectScript);
    QCOMPARE(page->selectedText().trimmed(), QString::fromLatin1("The quick brown fox"));

    // here the actions are enabled after a selection has been created
    QCOMPARE(page->action(QWebPage::SelectNextChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectPreviousChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectNextWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectPreviousWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectNextLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectPreviousLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectStartOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectEndOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectStartOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectEndOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectStartOfDocument)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectEndOfDocument)->isEnabled(), true);

    // make it editable before navigating the cursor
    page->setContentEditable(true);

    // cursor will be before the word "The", this makes sure there is a charet
    page->triggerAction(QWebPage::MoveToStartOfDocument);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // here the actions are enabled after contentEditable is true
    QCOMPARE(page->action(QWebPage::SelectNextChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectPreviousChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectNextWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectPreviousWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectNextLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectPreviousLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectStartOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectEndOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectStartOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectEndOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectStartOfDocument)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SelectEndOfDocument)->isEnabled(), true);

    delete page;
}

void tst_QWebPage::textEditing()
{
    CursorTrackedPage* page = new CursorTrackedPage;
    QString content("<html><body<p id=one>The quick brown fox</p>" \
        "<p id=two>jumps over the lazy dog</p>" \
        "<p>May the source<br/>be with you!</p></body></html>");
    page->mainFrame()->setHtml(content);

    // these actions must exist
    QVERIFY(page->action(QWebPage::Cut) != 0);
    QVERIFY(page->action(QWebPage::Copy) != 0);
    QVERIFY(page->action(QWebPage::Paste) != 0);
    QVERIFY(page->action(QWebPage::DeleteStartOfWord) != 0);
    QVERIFY(page->action(QWebPage::DeleteEndOfWord) != 0);
    QVERIFY(page->action(QWebPage::SetTextDirectionDefault) != 0);
    QVERIFY(page->action(QWebPage::SetTextDirectionLeftToRight) != 0);
    QVERIFY(page->action(QWebPage::SetTextDirectionRightToLeft) != 0);
    QVERIFY(page->action(QWebPage::ToggleBold) != 0);
    QVERIFY(page->action(QWebPage::ToggleItalic) != 0);
    QVERIFY(page->action(QWebPage::ToggleUnderline) != 0);
    QVERIFY(page->action(QWebPage::InsertParagraphSeparator) != 0);
    QVERIFY(page->action(QWebPage::InsertLineSeparator) != 0);
    QVERIFY(page->action(QWebPage::PasteAndMatchStyle) != 0);
    QVERIFY(page->action(QWebPage::RemoveFormat) != 0);
    QVERIFY(page->action(QWebPage::ToggleStrikethrough) != 0);
    QVERIFY(page->action(QWebPage::ToggleSubscript) != 0);
    QVERIFY(page->action(QWebPage::ToggleSuperscript) != 0);
    QVERIFY(page->action(QWebPage::InsertUnorderedList) != 0);
    QVERIFY(page->action(QWebPage::InsertOrderedList) != 0);
    QVERIFY(page->action(QWebPage::Indent) != 0);
    QVERIFY(page->action(QWebPage::Outdent) != 0);
    QVERIFY(page->action(QWebPage::AlignCenter) != 0);
    QVERIFY(page->action(QWebPage::AlignJustified) != 0);
    QVERIFY(page->action(QWebPage::AlignLeft) != 0);
    QVERIFY(page->action(QWebPage::AlignRight) != 0);

    // right now they are disabled because contentEditable is false
    QCOMPARE(page->action(QWebPage::Cut)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::Paste)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::DeleteStartOfWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::DeleteEndOfWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SetTextDirectionDefault)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SetTextDirectionLeftToRight)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::SetTextDirectionRightToLeft)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::ToggleBold)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::ToggleItalic)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::ToggleUnderline)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::InsertParagraphSeparator)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::InsertLineSeparator)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::PasteAndMatchStyle)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::RemoveFormat)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::ToggleStrikethrough)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::ToggleSubscript)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::ToggleSuperscript)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::InsertUnorderedList)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::InsertOrderedList)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::Indent)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::Outdent)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::AlignCenter)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::AlignJustified)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::AlignLeft)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::AlignRight)->isEnabled(), false);

    // Select everything
    page->triggerAction(QWebPage::SelectAll);

    // make sure it is enabled since there is a selection
    QCOMPARE(page->action(QWebPage::Copy)->isEnabled(), true);

    // make it editable before navigating the cursor
    page->setContentEditable(true);

    // clear the selection
    page->triggerAction(QWebPage::MoveToStartOfDocument);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // make sure it is disabled since there isn't a selection
    QCOMPARE(page->action(QWebPage::Copy)->isEnabled(), false);

    // here the actions are enabled after contentEditable is true
    QCOMPARE(page->action(QWebPage::Paste)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::DeleteStartOfWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::DeleteEndOfWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SetTextDirectionDefault)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SetTextDirectionLeftToRight)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::SetTextDirectionRightToLeft)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::ToggleBold)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::ToggleItalic)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::ToggleUnderline)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::InsertParagraphSeparator)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::InsertLineSeparator)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::PasteAndMatchStyle)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::ToggleStrikethrough)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::ToggleSubscript)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::ToggleSuperscript)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::InsertUnorderedList)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::InsertOrderedList)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::Indent)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::Outdent)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::AlignCenter)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::AlignJustified)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::AlignLeft)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::AlignRight)->isEnabled(), true);
    
    // make sure these are disabled since there isn't a selection
    QCOMPARE(page->action(QWebPage::Cut)->isEnabled(), false);
    QCOMPARE(page->action(QWebPage::RemoveFormat)->isEnabled(), false);
    
    // make sure everything is selected
    page->triggerAction(QWebPage::SelectAll);
    
    // this is only true if there is an editable selection
    QCOMPARE(page->action(QWebPage::Cut)->isEnabled(), true);
    QCOMPARE(page->action(QWebPage::RemoveFormat)->isEnabled(), true);

    delete page;
}

void tst_QWebPage::requestCache()
{
    TestPage page;
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.mainFrame()->setUrl(QString("data:text/html,<a href=\"data:text/html,Reached\" target=\"_blank\">Click me</a>"));
    QTRY_COMPARE(loadSpy.count(), 1);
    QTRY_COMPARE(page.navigations.count(), 1);

    page.mainFrame()->setUrl(QString("data:text/html,<a href=\"data:text/html,Reached\" target=\"_blank\">Click me2</a>"));
    QTRY_COMPARE(loadSpy.count(), 2);
    QTRY_COMPARE(page.navigations.count(), 2);

    page.triggerAction(QWebPage::Stop);
    QVERIFY(page.history()->canGoBack());
    page.triggerAction(QWebPage::Back);

    QTRY_COMPARE(loadSpy.count(), 3);
    QTRY_COMPARE(page.navigations.count(), 3);
    QCOMPARE(page.navigations.at(0).request.attribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork).toInt(),
             (int)QNetworkRequest::PreferNetwork);
    QCOMPARE(page.navigations.at(1).request.attribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork).toInt(),
             (int)QNetworkRequest::PreferNetwork);
    QCOMPARE(page.navigations.at(2).request.attribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork).toInt(),
             (int)QNetworkRequest::PreferCache);
}

void tst_QWebPage::backActionUpdate()
{
    QWebView view;
    QWebPage *page = view.page();
    QAction *action = page->action(QWebPage::Back);
    QVERIFY(!action->isEnabled());
    QSignalSpy loadSpy(page, SIGNAL(loadFinished(bool)));
    QUrl url = QUrl("qrc:///frametest/index.html");
    page->mainFrame()->load(url);
    QTRY_COMPARE(loadSpy.count(), 1);
    QVERIFY(!action->isEnabled());
    QTest::mouseClick(&view, Qt::LeftButton, 0, QPoint(10, 10));
    QTRY_COMPARE(loadSpy.count(), 2);

    QVERIFY(action->isEnabled());
}

void frameAtHelper(QWebPage* webPage, QWebFrame* webFrame, QPoint framePosition)
{
    if (!webFrame)
        return;

    framePosition += QPoint(webFrame->pos());
    QList<QWebFrame*> children = webFrame->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        if (children.at(i)->childFrames().size() > 0)
            frameAtHelper(webPage, children.at(i), framePosition);

        QRect frameRect(children.at(i)->pos() + framePosition, children.at(i)->geometry().size());
        QVERIFY(children.at(i) == webPage->frameAt(frameRect.topLeft()));
    }
}

void tst_QWebPage::frameAt()
{
    QWebView webView;
    QWebPage* webPage = webView.page();
    QSignalSpy loadSpy(webPage, SIGNAL(loadFinished(bool)));
    QUrl url = QUrl("qrc:///frametest/iframe.html");
    webPage->mainFrame()->load(url);
    QTRY_COMPARE(loadSpy.count(), 1);
    frameAtHelper(webPage, webPage->mainFrame(), webPage->mainFrame()->pos());
}

// import a little DRT helper function to trigger the garbage collector
void QWEBKIT_EXPORT qt_drt_garbageCollector_collect();

void tst_QWebPage::protectBindingsRuntimeObjectsFromCollector()
{
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));

    PluginPage* newPage = new PluginPage(m_view);
    m_view->setPage(newPage);

    m_view->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    m_view->setHtml(QString("<html><body><object type='application/x-qt-plugin' classid='lineedit' id='mylineedit'/></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 1);

    newPage->mainFrame()->evaluateJavaScript("function testme(text) { var lineedit = document.getElementById('mylineedit'); lineedit.setText(text); lineedit.selectAll(); }");

    newPage->mainFrame()->evaluateJavaScript("testme('foo')");

    qt_drt_garbageCollector_collect();

    // don't crash!
    newPage->mainFrame()->evaluateJavaScript("testme('bar')");
}

QTEST_MAIN(tst_QWebPage)
#include "tst_qwebpage.moc"
