wdns_res
wdns_downcase_rdata(wdns_rdata_t *rdata, uint16_t rrtype, uint16_t rrclass)
{

#define advance_bytes(x) do { \
	if (bytes_remaining < (x)) \
		return (wdns_res_parse_error); \
	p += (x); \
	bytes_remaining -= (x); \
} while(0)

	const record_descr *descr;
	const uint8_t *t;
	size_t bytes_remaining = rdata->len;
	uint8_t oclen;
	uint8_t *p = rdata->data;

	if (rrtype < record_descr_len)
		descr = &record_descr_array[rrtype];

	if (rrtype >= record_descr_len || descr->types[0] == rdf_unknown)
		return (wdns_res_success);

	/* only downcase rrtypes specified by RFC 4034 section 6.2
	 * and draft-ietf-dnsext-dnssec-bis-updates-11 section 5.1 */
	switch (rrtype) {
	case WDNS_TYPE_A6:
	case WDNS_TYPE_AFSDB:
	case WDNS_TYPE_CNAME:
	case WDNS_TYPE_DNAME:
	case WDNS_TYPE_KX:
	case WDNS_TYPE_MB:
	case WDNS_TYPE_MD:
	case WDNS_TYPE_MF:
	case WDNS_TYPE_MG:
	case WDNS_TYPE_MINFO:
	case WDNS_TYPE_MR:
	case WDNS_TYPE_MX:
	case WDNS_TYPE_NAPTR:
	case WDNS_TYPE_NS:
	case WDNS_TYPE_NXT:
	case WDNS_TYPE_PTR:
	case WDNS_TYPE_PX:
	case WDNS_TYPE_RP:
	case WDNS_TYPE_RT:
	case WDNS_TYPE_SIG:
	case WDNS_TYPE_SOA:
	case WDNS_TYPE_SRV:
		break;
	default:
		return (wdns_res_success);
	}

	if (descr->record_class == class_un ||
	    descr->record_class == rrclass)
	{
		for (t = &descr->types[0]; *t != rdf_end; t++) {
			if (bytes_remaining == 0)
				break;

			switch (*t) {
			case rdf_name:
			case rdf_uname:
				while (bytes_remaining-- != 0) {
					if (*p == 0) {
						p++;
						break;
					}
					if (*p >= 'A' && *p <= 'Z')
						*p |= 0x20;
					p++;
				}
				break;

			case rdf_repstring:
			case rdf_bytes:
			case rdf_bytes_b64:
			case rdf_type_bitmap:
				return (wdns_res_success);

			case rdf_int8:
				advance_bytes(1U);
				break;

			case rdf_int16:
			case rdf_rrtype:
				advance_bytes(2U);
				break;

			case rdf_int32:
			case rdf_ipv4:
				advance_bytes(4U);
				break;

			case rdf_ipv6:
				advance_bytes(16U);
				break;

			case rdf_string:
			case rdf_salt:
			case rdf_hash:
				oclen = *p;
				advance_bytes(oclen + 1U);
				break;

			case rdf_ipv6prefix:
				oclen = *p;
				if (oclen > 16U)
					return (wdns_res_parse_error);
				advance_bytes(oclen + 1U);
				break;

			default:
				fprintf(stderr, "ERROR: unhandled rdf type %u\n", *t);
				abort();
			}

		}
		if (bytes_remaining != 0) {
			return (wdns_res_parse_error);
		}
	}

	return (wdns_res_success);
}
