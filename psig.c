// psig.c ... functions on page signatures (psig's)
// part of SIMC signature files
// Written by John Shepherd, March 2020

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "hash.h"
#include <stdlib.h>


Bits codeword(char *attr, int m, int k)
{
    Count nbits = 0;
    Bits codeword  = newBits(m);
    srand(hash_any(attr, strlen(attr)));
    while (nbits < k) {
        int i = rand() % m;
        if (!bitIsSet(codeword, i)) {
            setBit(codeword, i);
            nbits++;
        }
    }
    return codeword;
}

Bits makePageSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO

	// width of page signature
	Count mOfPageSignature = psigBits(r);
	Count kOfPageSignature = codeBits(r);

    Bits  pageSignature = newBits(mOfPageSignature);

    char **tupValues = tupleVals(r, t);
    for (Count i = 0; i < nAttrs(r); i++) {
        if (strncmp(tupValues[i], "?",1) != 0) {
            // 生成signature of tuple
            Bits tupleSignature = codeword(tupValues[i],
                                             mOfPageSignature, kOfPageSignature);
            orBits(pageSignature, tupleSignature);
            freeBits(tupleSignature);
        }
    }
    return pageSignature;
}

void findPagesUsingPageSigs(Query q)
{
	assert(q != NULL);
	//TODO
    Reln r = q->rel;
    Bits queryPageSignatureBits = makePageSig(r, q->qstring);
    unsetAllBits(q->pages);

    File pageSignatureFile = psigFile(r);
    // width of page signature
    Count mOfPageSignature = psigBits(r);
    // number of psig pages
    Count numberOfSignaturePage = nPsigPages(r);
    // max tuple signatures per page
    Count maxNumberOfSignaturePage =   maxPsigsPP(r);
    Count total = 0;
    Offset currentPosition = 0;

    for (Count pageId = 0; pageId < numberOfSignaturePage; pageId++) {
        Page currentPage = getPage(pageSignatureFile, pageId);
        for (Offset position = 0; position < pageNitems(currentPage); position++,total++) {
            Bits pageSignature = newBits(mOfPageSignature);
            getBits(currentPage, position, pageSignature);
            if (isSubset(queryPageSignatureBits, pageSignature)) {
                currentPosition = pageId * maxNumberOfSignaturePage + position;
                setBit(q->pages, currentPosition);
            }

            // free
            freeBits(pageSignature);

            q->nsigs++;
        }
        q->nsigpages++;
    }
}

