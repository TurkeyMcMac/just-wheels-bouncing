/^\/\*\*$/ {
	inside_doc_comment = 1;
	next;
}

/^ \*\/$/ {
	inside_doc_comment = 0;
	print("");
	next;
}

/^ \*$/ {
	if (inside_doc_comment) {
		print("");
	}
	next;
}

/^ \* / {
	if (inside_doc_comment) {
		print(substr($0, 4));
	}
	next;
}
