#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define ERR_NONE                 0
#define ERR_NO_LINE_MATCHING     1
#define ERR_TOOFEWARGS           2
#define ERR_TOOMANYARGS          3
#define ERR_TWOSTRINGSPLUSFILE   4
#define ERR_FOPEN                5
#define ERR_ABSDISTBELOWZERO     6
#define ERR_ABSDISTNOINT         7
#define ERR_RELDISTBELOWZERO     8
#define ERR_RELDISTNOINT         9
#define ERR_SORTINGINVALID      10
#define ERR_HEADLIMITBELOWZERO  11
#define ERR_HEADLIMITNOINT      12
#define ERR_TAILLIMITBELOWZERO  13
#define ERR_TAILLIMITNOINT      14
#define ERR_HIGHLIGHTINGINVALID 15
#define ERR_S2SLIMITING         16
#define ERR_S2SDSTPREFIX        17
#define ERR_S2SSORTING          18
#define ERR_INVERTNOFILTER      19

#define ERR_UNKNOWN            999

enum CompareMode {
   stringWithString,
   stringWithFile,
   stringWithStdin
};

enum SortingType {
   unsorted,                                        // -sS u
   randomized,                                      // -sS r
   byDistance,                                      // -sS d
   alphabetically                                   // -sS a
};

struct HeadTailTrimmer {
    bool        isHeadTrimmer;
    std::string delimitter;
};

struct HeadTailLimit {
    bool     isHeadLimit;
    long int lineCount;
};

struct LDRange {
   int first;
   int last;
};

struct LDResult {
   int                  distance;
   std::vector<LDRange> locations;
   std::string          string1;
};

struct Config {
   CompareMode                  compareMode;          //
   std::string			string1;              //
   std::string			string2;              //
   std::string                  string2preprocessed;  //
   std::string                  str1FileName;         // -f
   std::istream*                str1File;             // -f
   std::vector<HeadTailTrimmer> trimmerQueue;         // -cC
   bool                         ignoreCase;           // -i
   bool                         matchAnySubstring;    // -g
   bool                         matchOnlyWords;       // -w
   int                          filterMaxAbsDist;     // -a -u
   float                        filterMaxRelStr1Dist; // -r -u
   float                        filterMaxRelStr2Dist; // -R -u
   bool                         filterInvert;         // -v
   SortingType                  sorting;              // -sS
   bool                         sortingReverse;       // -sS
   std::vector<HeadTailLimit>   limitingQueue;        // -HT
   bool                         formatPrintDistance;  // -dD
   bool                         lockPrintDistance;    // -dD
   bool                         formatPrintRange;     // -b
   bool                         formatPrintMatches;   // -m
};
