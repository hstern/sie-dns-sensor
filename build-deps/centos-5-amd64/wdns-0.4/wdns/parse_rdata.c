/**
 * Parse the rdata component of a resource record.
 *
 * \param[out] rr resource record object whose ->rdata field will be populated
 * \param[in] p pointer to start of message
 * \param[in] eop end of message buffer
 * \param[in] rdata pointer to rdata
 * \param[in] rdlen
 */

wdns_res
_wdns_parse_rdata(wdns_rr_t *rr, const uint8_t *p, const uint8_t *eop,
		  const uint8_t *rdata, uint16_t rdlen)
{

#define advance_bytes(x) do { \
	if (src_bytes < ((signed) (x))) \
		return (wdns_res_parse_error); \
	src += (x); \
	src_bytes -= (x); \
} while (0)

#define copy_bytes(x) do { \
	if (src_bytes < (x)) \
		return (wdns_res_parse_error); \
	ubuf_append(u, src, x); \
	src += (x); \
	src_bytes -= (x); \
} while (0)

	ubuf *u;
	const record_descr *descr;
	const uint8_t *src;
	const uint8_t *t;
	ssize_t src_bytes;
	size_t len;
	uint8_t domain_name[WDNS_MAXLEN_NAME];
	uint8_t oclen;
	wdns_res res;

	u = ubuf_new();
	src = rdata;
	src_bytes = (ssize_t) rdlen;

	if (rr->rrtype < record_descr_len)
		descr = &record_descr_array[rr->rrtype];

	if (rr->rrtype >= record_descr_len || descr->types[0] == rdf_unknown) {
		/* unknown rrtype, treat generically */
		copy_bytes(src_bytes);
	} else if (descr->record_class == class_un ||
		   descr->record_class == rr->rrclass)
	{
		for (t = &descr->types[0]; *t != rdf_end; t++) {
			if (src_bytes == 0)
				break;

			switch (*t) {
			case rdf_name:
				res = wdns_unpack_name(p, eop, src, domain_name, &len);
				if (res != wdns_res_success)
					goto parse_error;
				src_bytes -= wdns_skip_name(&src, eop);
				if (src_bytes < 0) {
					res = wdns_res_out_of_bounds;
					goto parse_error;
				}
				ubuf_append(u, domain_name, len);
				break;

			case rdf_uname:
				res = wdns_copy_uname(p, eop, src, domain_name, &len);
				if (res != wdns_res_success)
					goto parse_error;
				advance_bytes(len);
				ubuf_append(u, domain_name, len);
				break;

			case rdf_bytes:
			case rdf_bytes_b64:
				copy_bytes(src_bytes);
				break;

			case rdf_int8:
				copy_bytes(1);
				break;

			case rdf_int16:
			case rdf_rrtype:
				copy_bytes(2);
				break;

			case rdf_int32:
			case rdf_ipv4:
				copy_bytes(4);
				break;

			case rdf_ipv6:
				copy_bytes(16);
				break;

			case rdf_string:
			case rdf_salt:
			case rdf_hash:
				oclen = *src;
				copy_bytes(oclen + 1);
				break;

			case rdf_repstring:
				while (src_bytes > 0) {
					oclen = *src;
					copy_bytes(oclen + 1);
				}
				break;

			case rdf_ipv6prefix:
				oclen = *src;
				if (oclen > 16U) {
					res = wdns_res_out_of_bounds;
					goto parse_error;
				}
				copy_bytes(oclen + 1);
				break;

			case rdf_type_bitmap: {
				uint8_t bitmap_len;

				while (src_bytes >= 2) {
					bitmap_len = *(src + 1);

					if (!(bitmap_len >= 1 && bitmap_len <= 32)) {
						res = wdns_res_out_of_bounds;
						goto parse_error;
					}

					if (bitmap_len <= (src_bytes - 2)) {
						copy_bytes(2 + bitmap_len);
					} else {
						res = wdns_res_out_of_bounds;
						goto parse_error;
					}
				}
				break;
			}

			default:
				fprintf(stderr, "%s: unhandled rdf type %u\n", __func__, *t);
				abort();
			}

		}
		if (src_bytes != 0) {
			res = wdns_res_out_of_bounds;
			goto parse_error;
		}
	} else {
		/* unknown rrtype, treat generically */
		copy_bytes(src_bytes);
	}

	/* load rr->rdata */
	len = ubuf_size(u);
	rr->rdata = malloc(sizeof(wdns_rdata_t) + len);
	if (rr->rdata == NULL) {
		ubuf_destroy(&u);
		return (wdns_res_malloc);
	}
	rr->rdata->len = len;
	memcpy(rr->rdata->data, ubuf_cstr(u), len);
	ubuf_destroy(&u);

	return (wdns_res_success);

parse_error:
	ubuf_destroy(&u);
	if (res == wdns_res_success)
		res = wdns_res_failure;
	return (res);

#undef advance_bytes
#undef copy_bytes
}
