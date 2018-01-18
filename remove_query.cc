/*
 * remove_args.cc
 *
 *  Created on: 2017年3月1日
 *      Author: xie
 */

#include "ts/ts.h"
#include "ts/remap.h"
#include "ink_defs.h"
#include "ts/apidefs.h"
#include <string.h>


// Constants
const char PLUGIN_NAME[] = "remove_query";

static int url_query_check(TSHttpTxn txnp);

TSReturnCode
TSRemapInit(TSRemapInterface *apiInfo, char *errBuf, int erroBufSize) {
	return TS_SUCCESS;
}

TSReturnCode
TSRemapNewInstance(int argc, char *argv[], void **instance, char *errBuf, int errBufSize) {
	return TS_SUCCESS;
}

/**
 * @brief Plugin instance deletion clean-up entry point.
 * @param plugin instance pointer.
 */
void
TSRemapDeleteInstance(void *instance) {
	TSDebug(PLUGIN_NAME, "no op");
}



static int
url_query_check(TSHttpTxn txnp) {
	TSMBuffer bufp;
	TSMLoc url_loc = nullptr;
	char *req_url, *comma;
	int url_length, new_length;
	char cache_key_url[8192] = {0};

	if (TSHttpTxnPristineUrlGet(txnp, &bufp, &url_loc) == TS_SUCCESS) {
		req_url = TSUrlStringGet(bufp, url_loc, &url_length);
		TSHandleMLocRelease(bufp, TS_NULL_MLOC, url_loc);

		if (req_url == nullptr || url_length <= 0) {
			return TS_SUCCESS;
		}

		comma = strchr(req_url, '?');

		if (comma != nullptr) {
			new_length = comma - req_url;
			if (new_length > 8191)
				new_length = 8191;

			char new_url[new_length + 1];
			memcpy(new_url, req_url, new_length);
			new_url[new_length] = '\0';

			snprintf(cache_key_url, 8192, "%s", new_url);
			DEBUG_LOG("Rewriting cache URL for %s to %s", req_url, cache_key_url);
			if (req_url != NULL) {
				TSfree(req_url);
			}

			// set the cache key.
			if (TS_SUCCESS != TSCacheUrlSet(txnp, cache_key_url, strlen(cache_key_url))) {
				DEBUG_LOG("failed to change the cache url to %s.", cache_key_url);
			}
		}
	}
	return TS_SUCCESS;
}


TSRemapStatus
TSRemapDoRemap(void *instance, TSHttpTxn txn, TSRemapRequestInfo *rri) {
	TSCont txn_contp;
	int method_len;
	const char *method;

	method = TSHttpHdrMethodGet(rri->requestBufp, rri->requestHdrp, &method_len);
	if (method == TS_HTTP_METHOD_PURGE) {
		return TSREMAP_NO_REMAP;
	}

	url_query_check(txn);
	return TSREMAP_NO_REMAP;
}

