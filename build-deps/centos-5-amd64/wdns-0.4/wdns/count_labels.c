/**
 * Count the number of labels in an uncompressed domain name.
 *
 * \param[in] name
 * \param[out] nlabels
 *
 * \return wdns_res_success
 * \return wdns_res_invalid_length_octet
 * \return wdns_res_name_overflow
 */

wdns_res
wdns_count_labels(wdns_name_t *name, size_t *nlabels)
{
	uint8_t c, *data;

	*nlabels = 0;
	data = name->data;

	while ((c = *data++) != 0) {
		if (c <= 63) {
			*nlabels += 1;
			data += c;
			if (data - name->data > name->len)
				return (wdns_res_name_overflow);
		} else {
			return (wdns_res_invalid_length_octet);
		}
	}

	return (wdns_res_success);
}
