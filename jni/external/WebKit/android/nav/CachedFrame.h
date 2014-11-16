/*
 * Copyright 2007, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CachedFrame_H
#define CachedFrame_H

#include "CachedNode.h"
#include "IntRect.h"
#include "SkFixed.h"
#include "wtf/Vector.h"

namespace WebCore {
    class Frame;
    class Node;
}

namespace android {

class CachedHistory;
class CachedRoot;

    // first node referenced by cache is always document
class CachedFrame {
public:
    enum Direction {
        UNINITIALIZED = -1,
        LEFT,
        RIGHT,
        UP,
        DOWN,
        DIRECTION_COUNT,
        DIRECTION_MASK = DIRECTION_COUNT - 1,
        UP_DOWN = UP & DOWN,  // mask and result
        RIGHT_DOWN = RIGHT & DOWN, // mask and result
    };
    enum Compare {
        UNDECIDED = -1,
        TEST_IS_BEST,
        REJECT_TEST 
    };
    enum CursorInit {
        CURSOR_UNINITIALIZED = -2,
        CURSOR_CLEARED = -1,
        CURSOR_SET = 0
    };
    CachedFrame() {}
    void add(CachedNode& node) { mCachedNodes.append(node); }
    void addFrame(CachedFrame& child) { mCachedFrames.append(child); }
    bool checkVisited(const CachedNode* , CachedFrame::Direction ) const;
    size_t childCount() { return mCachedFrames.size(); }
    void clearCursor();
    const CachedNode* currentCursor() const { return currentCursor(NULL); }
    const CachedNode* currentCursor(const CachedFrame** ) const;
    const CachedNode* currentFocus() const { return currentFocus(NULL); }
    const CachedNode* currentFocus(const CachedFrame** ) const;
    bool directionChange() const;
    const CachedNode* document() const { return mCachedNodes.begin(); }
    bool empty() const { return mCachedNodes.size() < 2; } // must have 1 past doc
    const CachedNode* findBestAt(const WebCore::IntRect& , int* best,
        bool* inside, const CachedNode** , const CachedFrame** , int* x,
        int* y, bool checkForHidden) const;
    const CachedFrame* findBestFrameAt(int x, int y) const;
    const CachedNode* findBestHitAt(const WebCore::IntRect& , 
        int* best, const CachedFrame** , int* x, int* y) const;
    void finishInit();
    CachedFrame* firstChild() { return mCachedFrames.begin(); }
    const CachedFrame* firstChild() const { return mCachedFrames.begin(); }
    void* framePointer() const { return mFrame; }
    CachedNode* getIndex(int index) { return index >= 0 ?
        &mCachedNodes[index] : NULL; }
    const CachedFrame* hasFrame(const CachedNode* node) const;
    void hideCursor();
    int indexInParent() const { return mIndexInParent; }
    void init(const CachedRoot* root, int index, WebCore::Frame* frame);
    const CachedFrame* lastChild() const { return &mCachedFrames.last(); }
    CachedNode* lastNode() { return &mCachedNodes.last(); }
    CachedFrame* lastChild() { return &mCachedFrames.last(); }
    /**
     * Find the next textfield/textarea
     * @param start Must be a CachedNode in this CachedFrame's tree, or
     *              null, in which case we start from the beginning.
     * @param framePtr  If not null, and a textfield/textarea is found, its
     *                  CachedFrame will be pointed to by this pointer.
     * @param includeTextAreas If true, will return the next textfield or area.
     *                         Otherwise it only considers textfields.
     * @return CachedNode* Next textfield (or area)
     */
    const CachedNode* nextTextField(const CachedNode* start,
        const CachedFrame** framePtr, bool includeTextAreas) const;
    const CachedFrame* parent() const { return mParent; }
    CachedFrame* parent() { return mParent; }
    bool sameFrame(const CachedFrame* ) const;
    void removeLast() { mCachedNodes.removeLast(); }
    void resetClippedOut();
    void setContentsSize(int width, int height) { mContents.setWidth(width);
        mContents.setHeight(height); }
    bool setCursor(WebCore::Frame* , WebCore::Node* , int x, int y);
    void setCursorIndex(int index) { mCursorIndex = index; }
    void setData();
    bool setFocus(WebCore::Frame* , WebCore::Node* , int x, int y);
    void setFocusIndex(int index) { mFocusIndex = index; }
    void setLocalViewBounds(const WebCore::IntRect& bounds) { mLocalViewBounds = bounds; }
    int size() { return mCachedNodes.size(); }
    const CachedNode* validDocument() const;
protected:
    struct BestData {
        WebCore::IntRect mNodeBounds;
        WebCore::IntRect mMouseBounds;
        int mDistance;
        int mSideDistance;
        int mMajorDelta; // difference of center of object
            // used only when leading and trailing edges contain another set of edges
        int mMajorDelta2; // difference of leading edge (only used when center is same)
        int mMajorButt; // checks for next cell butting up against or close to previous one
        int mWorkingDelta;
        int mWorkingDelta2;
        int mNavDelta;
        int mNavDelta2;
        const CachedFrame* mFrame;
        const CachedNode* mNode;
        SkFixed mWorkingOverlap;   // this and below are fuzzy answers instead of bools
        SkFixed mNavOverlap;
        SkFixed mPreferred;
        bool mCursorChild;
        bool mInNav;
        bool mNavOutside;
        bool mWorkingOutside;
        int bottom() const { return bounds().bottom(); }
        const WebCore::IntRect& bounds() const { return mNodeBounds; }
        bool canBeReachedByAnotherDirection();
        int height() const { return bounds().height(); }
        bool inOrSubsumesNav() const { return (mNavDelta ^ mNavDelta2) >= 0; }
        bool inOrSubsumesWorking() const { return (mWorkingDelta ^ mWorkingDelta2) >= 0; }
        int isContainer(BestData* );
        static SkFixed Overlap(int span, int left, int right);
        void reset() { mNode = NULL; }
        int right() const { return bounds().right(); }
        void setDistances();
        bool setDownDirection(const CachedHistory* );
        bool setLeftDirection(const CachedHistory* );
        bool setRightDirection(const CachedHistory* );
        bool setUpDirection(const CachedHistory* );
        void setNavInclusion(int left, int right);
        void setNavOverlap(int span, int left, int right);
        void setWorkingInclusion(int left, int right);
        void setWorkingOverlap(int span, int left, int right);
        int width() const { return bounds().width(); }
        int x() const { return bounds().x(); }
        int y() const { return bounds().y(); }
    };
    typedef const CachedNode* (CachedFrame::*MoveInDirection)(
        const CachedNode* test, const CachedNode* limit, BestData* bestData, 
        const CachedNode* focus) const;
    void adjustToTextColumn(int* delta) const;
    static bool CheckBetween(Direction , const WebCore::IntRect& bestRect, 
        const WebCore::IntRect& prior, WebCore::IntRect* result);
    bool checkBetween(BestData* , Direction );
    int compare(BestData& testData, const BestData& bestData, const 
        CachedNode* focus) const;
    void findClosest(BestData* , Direction original, Direction test,
        WebCore::IntRect* clip) const;
    int frameNodeCommon(BestData& testData, const CachedNode* test, 
        BestData* bestData, BestData* originalData, 
        const CachedNode* focus) const;
    int framePartCommon(BestData& testData, const CachedNode* test, 
        BestData* bestData, const CachedNode* focus) const;
    const CachedNode* frameDown(const CachedNode* test, const CachedNode* limit, 
        BestData* , const CachedNode* focus) const;
    const CachedNode* frameLeft(const CachedNode* test, const CachedNode* limit, 
        BestData* , const CachedNode* focus) const;
    const CachedNode* frameRight(const CachedNode* test, const CachedNode* limit, 
        BestData* , const CachedNode* focus) const;
    const CachedNode* frameUp(const CachedNode* test, const CachedNode* limit, 
        BestData* , const CachedNode* focus) const;
    int minWorkingHorizontal() const;
    int minWorkingVertical() const;
    int maxWorkingHorizontal() const;
    int maxWorkingVertical() const;
    bool moveInFrame(MoveInDirection , const CachedNode* test, BestData* best,
        const CachedNode* focus) const;
    const WebCore::IntRect& _navBounds() const;
    WebCore::IntRect mContents;
    WebCore::IntRect mLocalViewBounds;
    WebCore::IntRect mViewBounds;
    WTF::Vector<CachedNode> mCachedNodes;
    WTF::Vector<CachedFrame> mCachedFrames;
    void* mFrame; // WebCore::Frame*, used only to compare pointers
    CachedFrame* mParent;
    int mCursorIndex;
    int mFocusIndex;
    int mIndexInParent; // index within parent's array of children, or -1 if root
    const CachedRoot* mRoot;
private:
    CachedHistory* history() const;
#ifdef BROWSER_DEBUG
public:
        CachedNode* find(WebCore::Node* ); // !!! probably debugging only
        int mDebugIndex;
        int mDebugLoopbackOffset;
#endif
#if !defined NDEBUG || DUMP_NAV_CACHE 
public:
    class Debug {
public:
        Debug() { 
#if DUMP_NAV_CACHE
            mFrameName[0] = '\0'; 
#endif
#if !defined NDEBUG
            mInUse = true; 
#endif
        }
#if !defined NDEBUG
        ~Debug() { mInUse = false; }
        bool mInUse;
#endif
#if DUMP_NAV_CACHE
        CachedFrame* base() const;
        void print() const;
        bool validate(const CachedNode* ) const;
        char mFrameName[256];
#endif
    } mDebug;
#endif
};

}

#endif
