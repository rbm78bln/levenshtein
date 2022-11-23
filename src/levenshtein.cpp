#include "levenshtein.h"
#include "distance.h"

Config   config;
Distance levenshtein;
char     *binary;

std::string options = "f:c:C:a:r:R:s:S:H:T:migwuvdDbh";
std::string whitespaces = " \t_-+*<>|.:,;!\"�$%&/()=?{[]}\\@#'~^*�";

int strLower(int c)
{
 return std::tolower((unsigned char)c);
}

void preprocessString(std::string& str, LDRange* range = NULL) {
   std::string::size_type pos, first, last;
   std::vector<HeadTailTrimmer>::iterator tIt;

   first=0; last=str.length()-1;

   for(tIt = config.trimmerQueue.begin(); tIt!=config.trimmerQueue.end(); ++tIt) {
      if(tIt->isHeadTrimmer) {
         if((pos=str.rfind(tIt->delimitter, last))!=std::string::npos && (pos+tIt->delimitter.length())>first) {
            first=pos+tIt->delimitter.length();
         }
      }
      else { // ! tIt->isTailTrimmer
         if((pos=str.find(tIt->delimitter, first))!=std::string::npos && pos<last) {
            last=pos;
         }
      }
   }

   if(config.matchOnlyWords) {
      first = str.find_first_not_of(whitespaces, first);
      last  = str.find_last_not_of(whitespaces, last)+1;
   }

   if( last!=str.length()-1 ) str.erase(str.begin()+last, str.end());
   if( first!=0 )             str.erase(str.begin(), str.begin()+first);
   if(config.ignoreCase) std::transform(str.begin(), str.end(), str.begin(), strLower);
   if(range!=NULL) { range->first=first; range->last=last; }
}

bool isValidError(const int& nowDistance, const int& subStrLen) {
   bool result = true;

   if(config.filterMaxAbsDist>=0 && nowDistance>config.filterMaxAbsDist) result=false;

   if(config.filterMaxRelStr1Dist>static_cast<float>(-0.1)) {
      int filterMaxStr1AbsDist = static_cast<int>((static_cast<float>(subStrLen)*config.filterMaxRelStr1Dist)+static_cast<float>(0.5));
      if(nowDistance>filterMaxStr1AbsDist) result=false;
   }

   result=(config.filterInvert!=result);

   return result;
}

int compareToString2(LDResult& result) {
   result.locations.clear();
   LDRange preLocation;

   std::string string1 = result.string1;
   preprocessString(string1, &preLocation);
   string1 += " ";

   if(config.string2preprocessed.length()==0 && config.matchAnySubstring) {
      preLocation.last = preLocation.first-1;
      result.locations.push_back(preLocation);
      result.distance = 0;
      return ERR_NONE;
   }

   std::string::size_type str1Len = string1.length();
   std::string::size_type subStrStart; std::string::size_type subStrLen;
   std::string::size_type minSubStrLen; std::string::size_type maxSubStrLen;
   if(config.matchAnySubstring && str1Len!=0) {
      minSubStrLen = 1;
      maxSubStrLen = str1Len<config.string2preprocessed.length()?str1Len:config.string2preprocessed.length();
   } else {
      minSubStrLen = maxSubStrLen = str1Len;
   }

   int nowDistance;
   LDRange location;
   std::string::size_type nextWhiteSp;
   std::string::size_type nextWordStarts;
   std::string::size_type skipToWhiteSp;
   std::string::size_type skipToWordStart;
   std::string::size_type skipToNextWordEnd;
   std::string::size_type maxSubStrLenNow;
   std::string::size_type maxSubStrWordEnd;

   result.distance = 1+(str1Len>config.string2preprocessed.length()?str1Len:config.string2preprocessed.length());

   subStrStart=0;
   skipToNextWordEnd=std::string::npos;
   while(subStrStart<=str1Len-minSubStrLen) {

      maxSubStrLenNow=maxSubStrLen<str1Len-subStrStart?maxSubStrLen:str1Len-subStrStart;
      if(config.matchOnlyWords) {

         if((nextWhiteSp=string1.find_first_of(whitespaces, subStrStart))==std::string::npos)
            nextWhiteSp=string1.length();

         if((nextWordStarts=string1.find_first_not_of(whitespaces, nextWhiteSp))==std::string::npos)
            nextWordStarts=string1.length();

         if((maxSubStrWordEnd=string1.find_first_of(whitespaces, subStrStart+maxSubStrLenNow))!=std::string::npos)
            maxSubStrLenNow=maxSubStrWordEnd-subStrStart;
         else --maxSubStrLenNow;
      } else {
         nextWordStarts=nextWhiteSp=subStrStart+1;
      }
      subStrLen=nextWhiteSp-subStrStart;


      while(subStrLen<=maxSubStrLenNow) {

         if((nowDistance=levenshtein.LD(string1.substr(subStrStart, subStrLen).c_str(), config.string2preprocessed.c_str()))<=result.distance) {
            if(isValidError(nowDistance, subStrLen)) {
               if(nowDistance<result.distance) {
                  result.distance=nowDistance;
                  result.locations.clear();
               }
               location.first=preLocation.first+subStrStart;
               location.last=location.first+subStrLen-1;
               result.locations.push_back(location);
            }
         }

         if(config.matchOnlyWords) {
            skipToWhiteSp=string1.find_first_of(whitespaces, subStrStart+subStrLen);
            if(skipToWhiteSp==std::string::npos) {
               skipToWordStart=skipToNextWordEnd=skipToWhiteSp=string1.length();
            } else {
               skipToWordStart=string1.find_first_not_of(whitespaces, skipToWhiteSp);
               if(skipToWordStart==std::string::npos) {
                  skipToWordStart=skipToWhiteSp;
               } else {
                 skipToNextWordEnd=string1.find_first_of(whitespaces, skipToWordStart);
                 if(skipToNextWordEnd==std::string::npos) {
                     skipToNextWordEnd=string1.length();
                 }
               }
            }
            subStrLen=(skipToNextWordEnd-subStrStart)>subStrLen?skipToNextWordEnd-subStrStart:subStrLen+1;
         }
         else ++subStrLen;
      }

      subStrStart=nextWordStarts;
   }

   return result.locations.size()>0?ERR_NONE:ERR_NO_LINE_MATCHING;
}

void printResult(LDResult& result) {
   std::vector<LDRange>::iterator lIt;

   if(config.formatPrintMatches) {
      for(lIt = result.locations.begin(); lIt != result.locations.end(); ++lIt) {
         if(config.formatPrintDistance) printf("%i ", result.distance);
         if(config.formatPrintRange)    printf("%i-%i ", lIt->first+1, lIt->last+1);
         if(lIt->last>lIt->first)
            printf("%s\n", result.string1.substr(lIt->first, lIt->last-lIt->first+1).c_str());
         else
            printf("\n");
      }
      printf("\n");
   } else {
      if(config.formatPrintDistance) printf("%i ", result.distance);
      lIt = result.locations.begin();
      if(config.formatPrintRange) {
         if(lIt != result.locations.end()) {
            printf("%i-%i", lIt->first+1, lIt->last+1);
            ++lIt;
         }
         while(lIt != result.locations.end()) {
            if(config.formatPrintRange) printf(",%i-%i", lIt->first+1, lIt->last+1);
            ++lIt;
         }
         printf(" ");
      }
      printf("%s\n", result.string1.c_str());
   }
}

bool sortByDistance(const LDResult& a, const LDResult& b) {
   return a.distance<b.distance;
}

bool sortByAlpha(const LDResult& a, const LDResult& b) {
   return a.string1.compare(b.string1)<0;
}

void sortResults(std::vector<LDResult>& results) {
   timeval tv;
   switch(config.sorting) {
      case byDistance:
         std::sort(results.begin(), results.end(), sortByDistance);
         break;
      case alphabetically:
         std::sort(results.begin(), results.end(), sortByAlpha);
         break;
      case randomized:
         if(gettimeofday(&tv, NULL)!=0) tv.tv_sec=time(NULL);
         srand( tv.tv_sec+tv.tv_usec );
         std::random_shuffle(results.begin(), results.end());
         break;
      default: // nothing to be done in other cases
         break;
   }
}

void limitResults(std::vector<LDResult>& results) {
   std::vector<HeadTailLimit>::iterator lIt;
   std::vector<LDResult>::iterator rItSrc, rItDst;
   for(lIt = config.limitingQueue.begin(); lIt != config.limitingQueue.end() && results.size()>0; ++lIt) {
      if(lIt->lineCount<results.size()) {
         if(lIt->isHeadLimit==config.sortingReverse) {
            rItSrc=results.end()-lIt->lineCount-1;
            rItDst=results.begin();
            while(rItSrc != results.end()) {
               (*rItDst) = (*rItSrc);
               ++rItDst;   ++rItSrc;
            }
         }
         results.resize(lIt->lineCount);
      }
   }
}

void printResults(std::vector<LDResult>& results) {
   std::vector<LDResult>::iterator rIt;

   if(!config.sortingReverse)
      for(rIt = results.begin(); rIt != results.end(); ++rIt) printResult(*rIt);
   else
      for(rIt = results.end()-1; rIt+1 != results.begin(); --rIt) printResult(*rIt);
}

int doLevenshtein() {
   std::vector<LDResult> results; results.clear();
   LDResult              result;
   int                   result_value = ERR_NO_LINE_MATCHING;

   if(config.compareMode == stringWithString) {
      result.string1=config.string1;
      result_value = compareToString2(result);
      if(config.formatPrintRange || config.formatPrintMatches) {
         if(result_value == ERR_NONE) printResult(result);
      } else {
         printf("%u\n", result.distance);
      }
   }

   if(config.str1File!=NULL) {
      if(config.sorting==unsorted && (!config.sortingReverse) && config.limitingQueue.size()==0) {
         while(std::getline(*config.str1File, result.string1))
            if(compareToString2(result) == ERR_NONE) {
               result_value = ERR_NONE;
               printResult(result);
            }
      } else {
         while(std::getline(*config.str1File, result.string1))
            if(compareToString2(result) == ERR_NONE) results.push_back(result);
      }
   }

   if(results.size()>0) result_value = ERR_NONE;

   sortResults(results);
   limitResults(results);

   printResults(results);

   return result_value;
}

void printHelp(char *binary) {
   fprintf(stderr, "\n%s:\n\n", binary);
   fprintf(stderr, "usage: levenshtein {-(c delim|C delim|i|g|u|a maxd|r maxd|R maxd|v|s order|\n   S order|H count|T count|d|D|b|m type)} (-h|[-f source|string1] string2)\n\n");
   fprintf(stderr, "where:\n");
   fprintf(stderr, "   string1/2 : Strings between the Levenshtein-distance is calculated.\n");
   fprintf(stderr, "               Only the resulting distance will be displayed.\n");
   fprintf(stderr, "               Returns 0, iff the result is within the given\n");
   fprintf(stderr, "               filter-ranges (see below). Returns 1 if not.\n\n");
   fprintf(stderr, "   -f source : Read string1 from file 'source'.\n");
   fprintf(stderr, "               Then compare line by line with string2.\n");
   fprintf(stderr, "               Every line from 'source' that is not filtered (see below)\n");
   fprintf(stderr, "               will be printed to stdout in the given sorting order.\n");
   fprintf(stderr, "               If the result is not sorted by distance (see below) every\n");
   fprintf(stderr, "               line will be preceeded by its distance to 'string2' by\n");
   fprintf(stderr, "               default. This behaviour can be overridden by -d or -D.\n");
   fprintf(stderr, "               Iff at least one line matches the given filter-ranges,\n");
   fprintf(stderr, "               0 is returned. Returns 1 if none matches.\n\n");
   fprintf(stderr, "               If string1 and -f are omitted, then stdin is used.\n\n");
   fprintf(stderr, "  preprocessing:\n");
   fprintf(stderr, "   -c delim  : Only use those parts of both strings that appear after\n");
   fprintf(stderr, "               the last occurence of 'delim'. If 'delim' does not appear\n");
   fprintf(stderr, "               in one string, then don't cut anything from the beginning.\n\n");
   fprintf(stderr, "   -C delim  : Only use those parts of both strings that appear before\n");
   fprintf(stderr, "               the first occurence of 'delim'. If 'delim' does not appear\n");
   fprintf(stderr, "               in one string, then don't cut anything from the end.\n\n");
   fprintf(stderr, "   -i        : ignore case.\n\n");
   fprintf(stderr, "   -g        : grep style. Match 'string2' with any substring of 'string1'\n");
   fprintf(stderr, "               or lines from 'source'. This may take some time!\n\n");
   fprintf(stderr, "   -w        : Match only with one ore more whole words. Implies -g. This\n");
   fprintf(stderr, "               often gives more reasonable results and works much faster.\n\n");
   fprintf(stderr, "  filtering:\n");
   fprintf(stderr, "   -u        : unfiltered. (default) Clear all previously defined filters. Any\n");
   fprintf(stderr, "               line will be printed and the result code will always be 0.\n\n");
   fprintf(stderr, "   -a maxd   : absolute distance. Only distances less or equal to 'maxd'\n");
   fprintf(stderr, "               are considered to be valid and will result in a return code\n");
   fprintf(stderr, "               of 0. All lines with distances above 'maxd' will be suppressed.\n\n");
   fprintf(stderr, "   -r maxd   : relative distance to length of 'string1' or 'source' in percent.\n");
   fprintf(stderr, "               Same as -a, but 'maxd' is determined by the length of 'string1'\n");
   fprintf(stderr, "               or the length of the current line from 'source'. E.g. if\n");
   fprintf(stderr, "               'string1' has a length of 10 characters and 'maxd' is given 30%%,\n");
   fprintf(stderr, "               then the maximum valid Levenshtein-distance would be 3.\n");
   fprintf(stderr, "               If used with -g, then 'maxd' is relative to the currently\n");
   fprintf(stderr, "               checked substring, not the whole string!\n\n");
   fprintf(stderr, "   -R maxd   : relative distance to length of 'string2' in percent.\n");
   fprintf(stderr, "               Same as -r, but 'maxd' referes to the length of 'string2'.\n\n");
   fprintf(stderr, "               -r and -R are taken from the length of the corresponding\n");
   fprintf(stderr, "               strings AFTER preprocessing!\n\n");
   fprintf(stderr, "   -v        : inverse. All filters are inverted. Result codes too.\n\n");
   fprintf(stderr, "  sorting:     only for use with -f.\n");
   fprintf(stderr, "   -s order  : Sorts the results by the given order ascendingly.\n");
   fprintf(stderr, "   -S order  : Sorts the results by the given order descendingly.\n");
   fprintf(stderr, "               where 'order' is one of:\n");
   fprintf(stderr, "                  u : unsorted   Does not reorder items. (default)\n");
   fprintf(stderr, "                  d : distance   Orders items according to their distance.\n");
   fprintf(stderr, "                  a : alpha      Orders items alphabetically.\n");
   fprintf(stderr, "                  r : randomize  Reorders items by chance.\n\n");
   fprintf(stderr, "  limiting:    only for use with -f.\n");
   fprintf(stderr, "   -H count  : Only display the first 'count' lines of the resulting list.\n");
   fprintf(stderr, "   -T count  : Only display the last 'count' lines of the resulting list.\n");
   fprintf(stderr, "               Both can be combined and even repeated, but mind their order!\n");
   fprintf(stderr, "               e.g. -H 2 -T 1 displays the second best result.\n\n");
   fprintf(stderr, "  format:      only for use with -f.\n");
   fprintf(stderr, "   -d        : Force preceeding any output line with its distance to 'string2'.\n");
   fprintf(stderr, "   -D        : Prevent preceeding any output line with its distance.\n\n");
   fprintf(stderr, "  more format: implies -g.\n");
   fprintf(stderr, "   -b        : Print the byte-range of the best matching substrings as 2nd item.\n");
   fprintf(stderr, "   -m        : Print only the best matches instead of the whole line.\n");
   fprintf(stderr, "   -h        : Displays this cruft.\n\n");
}

int errorMsg(char *binary, int error) {
   if(config.compareMode==stringWithFile && config.str1File!=NULL) { delete config.str1File; config.str1File=NULL; }

   if(error!=ERR_NONE && error!=ERR_NO_LINE_MATCHING) fprintf(stderr, "%s: ", binary);
   switch(error) {
      case -SIGHUP:
         fprintf(stderr, "caught SIGHUP. terminating cleanly.\n");
         break;
      case -SIGINT:
         fprintf(stderr, "caught SIGINT. terminating cleanly.\n");
         break;
      case -SIGQUIT:
         fprintf(stderr, "caught SIGQUIT. terminating cleanly.\n");
         break;
      case -SIGABRT:
         fprintf(stderr, "caught SIGABRT. terminating cleanly.\n");
         break;
      case -SIGTERM:
         fprintf(stderr, "caught SIGTERM. terminating cleanly.\n");
         break;
      case -SIGUSR1:
         fprintf(stderr, "caught SIGUSR1. terminating cleanly.\n");
         break;
      case -SIGUSR2:
         fprintf(stderr, "caught SIGUSR2. terminating cleanly.\n");
         break;
      case ERR_NONE:			// Everything is alright
         break;
      case ERR_NO_LINE_MATCHING:	// No error, but requirements are not met
         break;
      case ERR_TOOFEWARGS:
         fprintf(stderr, "not enough arguments. Try -h for help.\n");
         break;
      case ERR_TOOMANYARGS:
         fprintf(stderr, "too many arguments. You a most compare two strings.\n");
         break;
      case ERR_TWOSTRINGSPLUSFILE:
         fprintf(stderr, "too many arguments. Your can either specify 'string1' or an input file.\n");
         break;
      case ERR_FOPEN:
         fprintf(stderr, "error opening input file.\n");
         break;
      case ERR_ABSDISTBELOWZERO:
         fprintf(stderr, "maximum allowed absolute distance must be greater or equal to zero.\n");
         break;
      case ERR_ABSDISTNOINT:
         fprintf(stderr, "maximum allowed absolute distance must be a pure unsinged integer.\n");
         break;
      case ERR_RELDISTBELOWZERO:
         fprintf(stderr, "maximum allowed relative distance must be greater or equal to zero.\n");
         break;
      case ERR_RELDISTNOINT:
         fprintf(stderr, "maximum allowed relative distance must be a pure unsinged integer.\n");
         break;
      case ERR_SORTINGINVALID:
         fprintf(stderr, "invalid sortig method. choose one of u d a r.\n");
         break;
      case ERR_HEADLIMITBELOWZERO:
         fprintf(stderr, "head limiting must be above zero.\n");
         break;
      case ERR_HEADLIMITNOINT:
         fprintf(stderr, "head limiting must be a pure unsigned integer.\n");
         break;
      case ERR_TAILLIMITBELOWZERO:
         fprintf(stderr, "tail limiting must be above zero.\n");
         break;
      case ERR_TAILLIMITNOINT:
         fprintf(stderr, "tail limiting must be a pure unsigned integer.\n");
         break;
      case ERR_HIGHLIGHTINGINVALID:
         fprintf(stderr, "invalid highlighting method. choose one of b s u n.\n");
         break;
      case ERR_S2SLIMITING:
         fprintf(stderr, "head and tail limiting makes only sense with -f.\n");
         break;
      case ERR_S2SDSTPREFIX:
         fprintf(stderr, "-d and -D are only valid when combined with -f, -b, or -m.\n");
         break;
      case ERR_S2SSORTING:
         fprintf(stderr, "-s and -S are only valid when combined with -f.\n\nYou cannot sort a single line, dude!\n");
         break;
      case ERR_INVERTNOFILTER:
         fprintf(stderr, "-v is only valid with -a, -r, or -R.\n\nYou cannot invert no filter, dude!\n");
         break;
      default:
         fprintf(stderr, "unknown error %u.\n", error);
         break;
   }

   return error;
}

void initConfig() {
   config.compareMode = stringWithString;
   config.string1.clear();
   config.string2.clear();
   config.str1FileName.clear();
   config.str1File = NULL;
   config.trimmerQueue.clear();
   config.ignoreCase = false;
   config.matchAnySubstring = false;
   config.matchOnlyWords = false;
   config.filterMaxAbsDist = -1;
   config.filterMaxRelStr1Dist = -1.0;
   config.filterMaxRelStr2Dist = -1.0;
   config.filterInvert = false;
   config.sorting = unsorted;
   config.sortingReverse = false;
   config.limitingQueue.clear();
   config.formatPrintDistance = true;
   config.lockPrintDistance = false;
   config.formatPrintRange = false;
   config.formatPrintMatches = false;
}

int validateConfig(int argc, int optind, char** argv) {
   int argcRemain = argc - optind;
   if(argcRemain<=0) return ERR_TOOFEWARGS;
   if(argcRemain>2) return ERR_TOOMANYARGS;
   if(argcRemain==2 && config.str1FileName.length()>0) return ERR_TWOSTRINGSPLUSFILE;

   if(config.str1FileName.compare("-")==0) config.str1FileName.clear();
   if(argcRemain==1) {
      config.compareMode = stringWithStdin;
      if(config.str1FileName.length()>0) {
         config.str1File = new std::ifstream(config.str1FileName.c_str(), std::ios::in);
         if(!*config.str1File) {
            delete config.str1File;
            config.str1File = NULL;
            return ERR_FOPEN;
         }
         config.compareMode = stringWithFile;
      } else config.str1File = &std::cin;
      config.string1.clear();
      config.string2 = std::string(argv[optind]);
   } else {
      config.str1File = NULL;
      config.string1 = std::string(argv[optind]);
      config.string2 = std::string(argv[optind+1]);
      config.compareMode = stringWithString;
   }

   if(config.compareMode == stringWithString) {
      if(config.limitingQueue.size()>0) return ERR_S2SLIMITING;
      if(config.sorting != unsorted)    return ERR_S2SSORTING;
      if(config.lockPrintDistance && !(config.formatPrintMatches || config.formatPrintRange))
                                        return ERR_S2SDSTPREFIX;
   }

   for(int i=config.trimmerQueue.size()-1; i>=0; --i) {
      if(config.trimmerQueue[i].delimitter.length()==0) {
         config.trimmerQueue.erase(config.trimmerQueue.begin()+i);
      }
   }

   config.string2preprocessed=config.string2;
   preprocessString(config.string2preprocessed);

   if(config.filterMaxRelStr2Dist>static_cast<float>(-0.1)) {
      int filterMaxStr2AbsDist = static_cast<int>((static_cast<float>(config.string2preprocessed.length())*config.filterMaxRelStr2Dist)+static_cast<float>(0.5));
      if(config.filterMaxAbsDist < 0 || config.filterMaxAbsDist > filterMaxStr2AbsDist) config.filterMaxAbsDist = filterMaxStr2AbsDist;
   }
   config.filterMaxRelStr2Dist = static_cast<float>(-1.0);

   if(config.filterInvert && config.filterMaxAbsDist<0 && config.filterMaxRelStr1Dist<static_cast<float>(0.0))
      return ERR_INVERTNOFILTER;

   return ERR_NONE;
}

int setStr1FileName(char* optarg) {
   config.str1FileName = std::string(optarg);
   return ERR_NONE;
}

int setPreStartFromHere(char* optarg) {
   HeadTailTrimmer trimmer;
   trimmer.isHeadTrimmer = true;
   trimmer.delimitter = std::string(optarg);
   config.trimmerQueue.push_back(trimmer);
   return ERR_NONE;
}

int setPreEndHere(char* optarg) {
   HeadTailTrimmer trimmer;
   trimmer.isHeadTrimmer = false;
   trimmer.delimitter = std::string(optarg);
   config.trimmerQueue.push_back(trimmer);
   return ERR_NONE;
}

int setIgnoreCase() {
   config.ignoreCase = true;
   return ERR_NONE;
}

int setMatchAnySubstring() {
   config.matchAnySubstring = true;
   return ERR_NONE;
}

int setMatchOnlyWords() {
   setMatchAnySubstring();
   config.matchOnlyWords = true;
   return ERR_NONE;
}

int clearAnyFilters() {
   config.filterMaxAbsDist = -1;
   config.filterMaxRelStr1Dist = -1.0;
   config.filterMaxRelStr2Dist = -1.0;
   config.filterInvert = false;
   return ERR_NONE;
}

int setFilterMaxAbsDist(char* optarg) {
   int i;
   if( sscanf(optarg, "%u", &i) > 0 ) {
      if(i>=0) {
         if(i<config.filterMaxAbsDist || config.filterMaxAbsDist<0) config.filterMaxAbsDist = i;
         return ERR_NONE;
      } else return ERR_ABSDISTBELOWZERO;
   } else return ERR_ABSDISTNOINT;
   return ERR_UNKNOWN;
}

int setFilterMaxRelStr1Dist(char* optarg) {
   int i;
   float f;
   if( sscanf(optarg, "%u", &i) > 0 ) {
      if(i>=0) {
         f=(static_cast<float>(i)/static_cast<float>(100));
         if(f<config.filterMaxRelStr1Dist || config.filterMaxRelStr1Dist<static_cast<float>(0.1)) config.filterMaxRelStr1Dist = f;
         return ERR_NONE;
      } else return ERR_RELDISTBELOWZERO;
   } else return ERR_RELDISTNOINT;
   return ERR_UNKNOWN;
}

int setFilterMaxRelStr2Dist(char* optarg) {
   int i;
   float f;
   if( sscanf(optarg, "%u", &i) > 0 ) {
      if(i>=0) {
         f=(static_cast<float>(i)/static_cast<float>(100));
         if(f<config.filterMaxRelStr2Dist || config.filterMaxRelStr2Dist<static_cast<float>(0.1)) config.filterMaxRelStr2Dist = f;
         return ERR_NONE;
      } else return ERR_RELDISTBELOWZERO;
   } else return ERR_RELDISTNOINT;
   return ERR_UNKNOWN;
}

int setFilterInvese() {
   config.filterInvert = true;
   return ERR_NONE;
}

int setSorting(char* optarg) {
   switch(optarg[0]) {
      case 'u':
         config.sorting = unsorted;
         break;
      case 'd':
         config.sorting = byDistance;
         break;
      case 'a':
         config.sorting = alphabetically;
         break;
      case 'r':
         config.sorting = randomized;
         break;
      default:
         return ERR_SORTINGINVALID;
         break;
   }
   config.sortingReverse = false;
   if(!config.lockPrintDistance) config.formatPrintDistance = (config.sorting != byDistance);
   return ERR_NONE;
}

int setSortingReverse(char* optarg) {
   int result = setSorting(optarg);
   config.sortingReverse = true;
   return result;
}

int addLimitingHead(char* optarg) {
   int i;
   HeadTailLimit limit;
   if( sscanf(optarg, "%u", &i) > 0 ) {
      if(i>=0) {
         limit.isHeadLimit = true;
         limit.lineCount = i;
         config.limitingQueue.push_back(limit);
         return ERR_NONE;
      } else return ERR_HEADLIMITBELOWZERO;
   } else return ERR_HEADLIMITNOINT;
   return ERR_UNKNOWN;
}

int addLimitingTail(char* optarg) {
   int i;
   HeadTailLimit limit;
   if( sscanf(optarg, "%u", &i) > 0 ) {
      if(i>=0) {
         limit.isHeadLimit = false;
         limit.lineCount = i;
         config.limitingQueue.push_back(limit);
         return ERR_NONE;
      } else return ERR_TAILLIMITBELOWZERO;
   } else return ERR_TAILLIMITNOINT;
   return ERR_UNKNOWN;
}

int setFormatPrintDistance() {
   config.formatPrintDistance = true;
   config.lockPrintDistance = true;
   return ERR_NONE;
}

int clearFormatPrintDistance() {
   config.formatPrintDistance = false;
   config.lockPrintDistance = true;
   return ERR_NONE;
}

int setFormatPrintRange() {
   setMatchAnySubstring();
   config.formatPrintRange = true;
   return ERR_NONE;
}

int setPrintOnlyMatches() {
   setMatchAnySubstring();
   config.formatPrintMatches = true;
   return ERR_NONE;
}

void signalHandler(int sig) {
   exit(errorMsg(binary, -sig));
}

void installSignalHandler(char* argv0) {
   binary=argv0;
   sigset_t mask;
   struct sigaction action;
   sigemptyset( &mask );
   action.sa_mask = mask;
   action.sa_flags = SA_NODEFER;
   action.sa_handler = signalHandler;
   sigaction(SIGHUP , &action, NULL);
   sigaction(SIGINT , &action, NULL);
   sigaction(SIGQUIT, &action, NULL);
   sigaction(SIGABRT, &action, NULL);
   sigaction(SIGTERM, &action, NULL);
   sigaction(SIGUSR1, &action, NULL);
   sigaction(SIGUSR2, &action, NULL);
}

int main(int argc, char** argv) {
// installSignalHandler(argv[0]);
   initConfig();
   int optResult = 2;

   int option;
   while((option = getopt(argc, argv, options.c_str())) != -1) {
      switch(option) {
         case 'f':
            if((optResult=setStr1FileName(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'c':
            if((optResult=setPreStartFromHere(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'C':
            if((optResult=setPreEndHere(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'i':
            if((optResult=setIgnoreCase())>0) return errorMsg(argv[0], optResult);
            break;
         case 'g':
            if((optResult=setMatchAnySubstring())>0) return errorMsg(argv[0], optResult);
            break;
         case 'w':
            if((optResult=setMatchOnlyWords())>0) return errorMsg(argv[0], optResult);
            break;
         case 'u':
            if((optResult=clearAnyFilters())>0) return errorMsg(argv[0], optResult);
            break;
         case 'a':
            if((optResult=setFilterMaxAbsDist(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'r':
            if((optResult=setFilterMaxRelStr1Dist(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'R':
            if((optResult=setFilterMaxRelStr2Dist(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'v':
            if((optResult=setFilterInvese())>0) return errorMsg(argv[0], optResult);
            break;
         case 's':
            if((optResult=setSorting(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'S':
            if((optResult=setSortingReverse(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'H':
            if((optResult=addLimitingHead(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'T':
            if((optResult=addLimitingTail(optarg))>0) return errorMsg(argv[0], optResult);
            break;
         case 'd':
            if((optResult=setFormatPrintDistance())>0) return errorMsg(argv[0], optResult);
            break;
         case 'D':
            if((optResult=clearFormatPrintDistance())>0) return errorMsg(argv[0], optResult);
            break;
         case 'b':
            if((optResult=setFormatPrintRange())>0) return errorMsg(argv[0], optResult);
            break;
         case 'm':
            if((optResult=setPrintOnlyMatches())>0) return errorMsg(argv[0], optResult);
            break;
         case 'h':
            printHelp(argv[0]);
            return errorMsg(argv[0], ERR_NONE);
            break;
      }
   }

   if((optResult=validateConfig(argc, optind, argv))>0) return errorMsg(argv[0], optResult);

   return errorMsg(argv[0], doLevenshtein());
}
