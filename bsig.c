// bsig.c ... functions on Tuple Signatures (bsig's)
// part of SIMC signature files
// Written by John Shepherd, March 2020

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"

void findPagesUsingBitSlices(Query q) {
    assert(q != NULL);
    Reln r = q->rel;
    File bitSlicedSignatureFile = bsigFile(r);
    Bits queryPageSignature = makePageSig(r, q->qstring);

    PageID bitSlicedPageId = 0, tempBitSlicedPageId = 0;
    Page bitSlicedPage = NULL;
    Offset position = 0;

    // max bit-slices per page
    Count maxBitSlicedPerPage = maxBsigsPP(r);
    // width of bit-slice (=maxpages)
    Count mOfBitSlicedPage = bsigBits(r);
    // width of page signature (#bits)
    Count mOfPageSignature = psigBits(r);

    Bits queryPages = newBits(mOfBitSlicedPage);
    setAllBits(queryPages);

    for (Count index = 0; index < mOfPageSignature; index++) {
        if (bitIsSet(queryPageSignature, index)) {
            // 获取所在的bit sliced的位置
            tempBitSlicedPageId = index / maxBitSlicedPerPage;
            position = index % maxBitSlicedPerPage;
            if (bitSlicedPage == NULL || tempBitSlicedPageId != bitSlicedPageId){
                bitSlicedPageId  = tempBitSlicedPageId;
                bitSlicedPage = getPage(bitSlicedSignatureFile, bitSlicedPageId);
                q->nsigpages++;
            }

            Bits bsig = newBits(mOfBitSlicedPage);
            getBits(bitSlicedPage, position, bsig);
            // 遍历每一个属性
            for (Offset pos = 0; pos < mOfBitSlicedPage; pos++) {
                if (!bitIsSet(bsig, pos)) {
                    unsetBit(queryPages, pos);
                }
            }
            q->nsigs++;
            // free
            freeBits(bsig);
        }
    }

    q->pages = queryPages;
}
