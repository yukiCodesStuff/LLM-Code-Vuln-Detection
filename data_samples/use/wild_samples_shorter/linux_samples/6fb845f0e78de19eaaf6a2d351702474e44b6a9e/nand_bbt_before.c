
/**
 * read_bbt - [GENERIC] Read the bad block table starting from page
 * @chip: NAND chip object
 * @buf: temporary buffer
 * @page: the starting page
 * @num: the number of bbt descriptors to read
 * @td: the bbt describtion table