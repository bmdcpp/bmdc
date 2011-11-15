#if !defined(GZIP_H)
#define GZIP_H
#include "typedefs.h"

namespace dcpp {

class TarFile
{
	public:
		void CreateTarredFile(const string _path, const StringPairList& files);
		void DecompresTarredFile(const string _file, const string& _prefix);

};

}
#endif
