// tsig.c ... functions on Tuple Signatures (tsig's)
// part of SIMC signature files
// Written by John Shepherd, March 2020

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"
#include "util.h"

// helper function that generates codeword for tuple signatures

// 课件上老师的代码
Bits coword(char *attr, int m, int k) {
    Count nbits = 0;
    Bits result = newBits(m);
    srand(hash_any(attr, strlen(attr)));
    while (nbits < k) {
        int i = rand() % m;
        if (!bitIsSet(result, i)) {
            setBit(result, i);
            nbits++;
        }
    }
    return result;
}

// make a tuple signature

Bits makeTupleSig(Reln r, Tuple t) {
    assert(r != NULL && t != NULL);
    //TODO

    // width of tuple signature
    Count mOfTuple = tsigBits(r);
    // bits set per attribute
    Count kOfTuple = codeBits(r);
    // 每个tuple的签名
    Bits tupleSignature = newBits(mOfTuple);

    char **tupValues = tupleVals(r, t);
    for (Count index = 0; index < nAttrs(r); index++) {
        // 判断是否是？
        if (strncmp(tupValues[index], "?", 1) != 0) {
            Bits cw = coword(tupValues[index], mOfTuple, kOfTuple);
            orBits(tupleSignature, cw);
            // 释放内存
            free(cw);
        }
    }
    // 返回生成的Signature
    return tupleSignature;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q) {
    assert(q != NULL);
    //TODO

    Reln reln = q->rel;

    Bits querySignature = makeTupleSig(reln, q->qstring);
    unsetAllBits(q->pages);

    // width of tuple signature
    Count mOfTuple = tsigBits(reln);
    // number of tsig pages
    Count numberOfTupleSignaturePages = nTsigPages(q->rel);
    // max tuple signatures per page
    Count maxTupleSignaturePerPage = maxTsigsPP(q->rel);
    // max tuples per page
    Count maxTupleSizeOfPage = maxTupsPP(reln);

    // tuple的Signature file
    File tupleSignatureFile = tsigFile(q->rel);
    Offset currentPosition = 0;

    for (PageID currentPageId = 0; currentPageId < numberOfTupleSignaturePages; currentPageId++) {
        Page currentPage = getPage(tupleSignatureFile, currentPageId);

        for (Offset currentTupleIndex = 0; currentTupleIndex < pageNitems(currentPage); currentTupleIndex++) {
            // 获取对应的tuple的签名
            Bits tupleSignature = newBits(mOfTuple);
            getBits(currentPage, currentTupleIndex, tupleSignature);

            if (isSubset(querySignature, tupleSignature)) {
                Offset tupleId = currentPageId * maxTupleSignaturePerPage + currentTupleIndex;
                currentPosition = tupleId / maxTupleSizeOfPage;
                // set bit
                setBit(q->pages, currentPosition);
            }
            // free
            freeBits(tupleSignature);
            q->nsigs++;
        }
        // nsigpages ++
        q->nsigpages++;
    }
}
