{
	struct af_alg_completion *completion = req->data;

	if (err == -EINPROGRESS)
		return;

	completion->err = err;
	complete(&completion->completion);
}
EXPORT_SYMBOL_GPL(af_alg_complete);