#include "stdinc.h"
#include "DCPlusPlus.h"

#include "SSL.h"
#include "Util.h"

namespace dcpp {
namespace ssl {

/*std::string*/ vector<uint8_t> X509_digest(::X509* x509, const ::EVP_MD* md) {
	unsigned int n;
	unsigned char buf[EVP_MAX_MD_SIZE];

	if (!X509_digest(x509, md, buf, &n)) {
		//return Util::emptyString; // Throw instead?
		return vector<uint8_t>(); // Throw instead?
	}
	std::string ret(n * 2, '\0');
	for(unsigned int i = 0; i < n; ++i) {
		sprintf(&ret[i*2], "%02x", (unsigned int)buf[i]);
	}
	//return ret;
	return vector<uint8_t>(buf, buf+n);
}

}
}
