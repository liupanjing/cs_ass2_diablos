// query.c ... query scan functions
// part of SIMC signature files
// Manage creating and using Query objects
// Written by John Shepherd, March 2020

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"
#include "tsig.h"
#include "psig.h"
#include "bsig.h"

// check whether a query is valid for a relation
// e.g. same number of attributes

int checkQuery(Reln r, char *q)
{
	if (*q == '\0') return 0;
	char *c;
	int nattr = 1;
	for (c = q; *c != '\0'; c++)
		if (*c == ',') nattr++;
	return (nattr == nAttrs(r));
}

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q, char sigs)
{
	Query new = malloc(sizeof(QueryRep));
	assert(new != NULL);
	if (!checkQuery(r,q)) return NULL;
	new->rel = r;
	new->qstring = q;
	new->nsigs = new->nsigpages = 0;
	new->ntuples = new->ntuppages = new->nfalse = 0;
	new->pages = newBits(nPages(r));
	switch (sigs) {
	case 't': findPagesUsingTupSigs(new); break;
	case 'p': findPagesUsingPageSigs(new); break;
	case 'b': findPagesUsingBitSlices(new); break;
	default:  setAllBits(new->pages); break;
	}
	new->curpage = 0;
	return new;
}

// scan through selected pages (q->pages)
// search for matching tuples and show each
// accumulate query stats

void scanAndDisplayMatchingTuples(Query q)
{
	assert(q != NULL);
	Reln r = q -> rel;
	//写在哪个文件里
	File datFile = dataFile(r);
	Count totalPages = nPages(r);
	q ->curpage = 0;
	//开始遍历
	while(q ->curpage <totalPages){
		if(bitIsSet(q ->pages,q ->curpage) == FALSE)
			continue;
		//这个page有可能是我想要的
		if(bitIsSet(q -> pages , q)){
			Page cur_page = getPage(datFile,q->curpage);
			Count totalTuples = pageNitems(cur_page);
			q ->curtup = 0;
			Count matches = 0;
			while(q -> curtup < totalTuples){
				//获取每一个url
				Tuple cur_tuple = getTupleFromPage(r,cur_page,q->curtup);
				//如果匹配上
				if(tupleMatch(r,cur_tuple,q->qstring)){
					showTuple(r,cur_tuple);
					matches++;
				}
				q -> curpage++;
				q -> ntuples++;
			}
			if (matches == 0){
				q -> nfalse++;
			}
			//一共扫描多少page
			q -> ntuppages++;
		}
		q -> curpage++;
	}
}

// print statistics on query

void queryStats(Query q)
{
	printf("# sig pages read:    %d\n", q->nsigpages);
	printf("# signatures read:   %d\n", q->nsigs);
	printf("# data pages read:   %d\n", q->ntuppages);
	printf("# tuples examined:   %d\n", q->ntuples);
	printf("# false match pages: %d\n", q->nfalse);
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	free(q->pages);
	free(q);
}

