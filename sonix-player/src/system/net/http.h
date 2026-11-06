#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <stddef.h>

// Bounded HTTP requests for clock synchronization and optional scrobbling.
// Call only from worker threads. HTTPS retains certificate/hostname checks.
#define HTTP_USER_AGENT "SonixPlayer/1.0"

// Fetches `url` into a freshly malloc'd, NUL-terminated buffer. The caller
// frees it. `out_len` may be NULL. Redirects (301/302/303/307/308) are
// followed, up to HTTP_MAX_REDIRECTS, and may cross between http and https in
// either direction -- which real redirects do constantly.
//
// Returns false on any failure, leaving *out untouched. `limit` caps how much
// body is kept; a longer response is truncated rather than allowed to eat the
// heap.
#define HTTP_MAX_REDIRECTS 5
bool http_get(const char *url, char **out, size_t *out_len, size_t limit, int timeout_secs);

bool http_get_ex(const char *url, const char *extra_headers, bool want_error_body, char **out, size_t *out_len,
				 size_t limit, int timeout_secs, int *status_out);

// Where the request ended up, which is not always where it started: radio CDNs
// answer an HLS manifest with a 302 to an edge (msvdn.net does it every
// session), and the relative URLs inside the manifest resolve against the edge,
// not the requested host. After a successful http_get()/http_request() this
// holds the URL of the last hop -- per thread, like http_last_error(), and
// overwritten by the next request on the same thread, so copy it immediately.
const char *http_last_final_url(void);

typedef struct {
	// "GET", "POST", "DELETE". NULL means GET.
	const char *method;

	// Lines already terminated with "\r\n", one per header, or NULL.
	const char *extra_headers;

	// The body, or NULL. `body_len` 0 with a non-NULL body means strlen(),
	// which is the normal case since these bodies are text.
	const char *body;
	size_t body_len;

	const char *content_type;

	// Keep the body of a 4xx instead of discarding it: that is where the API
	// says why it refused. See http_get_ex().
	bool want_error_body;

	// Read success from the status code rather than the content. Two effects,
	// both needed by the same kind of caller:
	//
	//   * any 2xx counts as success, not just 200. Creating something answers
	//     201, which without this would read as a failure;
	//   * an empty body counts as a response. A successful DELETE answers 204
	//     and sends nothing; and a 4xx with no body -- which happens -- would
	//     otherwise surface as "no response" instead of its code, hiding a 401
	//     from a caller waiting for it to refresh the token.
	//
	// With this set *out is still allocated and terminated: an empty string,
	// never NULL, so the caller always frees the same way.
	bool allow_empty_body;

	// Opt-in: fail before sending any request over plain HTTP, including redirects.
	bool https_only;

} http_req_t;

// Like http_get_ex(), but with the method and body chosen by the caller.
// Redirects are followed as usual; a 303, and a 301/302 on a POST, become a GET
// to the new destination, which is what browsers do and what servers expect.
bool http_request(const char *url, const http_req_t *req, char **out, size_t *out_len, size_t limit, int timeout_secs,
				  int *status_out);

#define HTTP_HEADER_BUFFER 8192

// Why the last HTTP request on THIS thread failed, when
// the reason is one a person could act on -- a device clock so wrong that
// every certificate looks expired, a missing certificate bundle, a name that
// does not match. NULL when there is nothing useful to say, which is the usual
// case for an ordinary network failure.
//
// Per thread on purpose: the radio's directory worker and its playback thread
// both use this file and must not overwrite each other's answer.
const char *http_last_error(void);

// Percent-encodes `in` into `out` for use inside a URL path segment or query
// value. Returns out.
char *http_url_encode(const char *in, char *out, size_t out_size);

#endif /* HTTP_H */
