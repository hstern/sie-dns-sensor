/**
 * Downcase a wdns_name_t.
 *
 * \param[in] name the name to downcase
 */

void
wdns_downcase_name(wdns_name_t *name)
{
	uint8_t *p = name->data;
	uint16_t len = name->len;

	while (len-- != 0) {
		if (*p >= 'A' && *p <= 'Z')
			*p |= 0x20;
		p++;
	}
}
