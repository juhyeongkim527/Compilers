fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", name, lineno);
fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", name, lineno);
fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", lineno, name);
fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indicies should be integer\n", lineno, name);
fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", lineno, name);
fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", lineno, name);
fprintf(listing, "Error: Invalid return at line %d\n", lineno);
fprintf(listing, "Error: invalid assignment at line %d\n", lineno);
fprintf(listing, "Error: invalid operation at line %d\n", lineno);
fprintf(listing, "Error: invalid condition at line %d\n", lineno);
fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line ", name, lineno);

// Program to sequentially print all the line numbers

fprintf(listing, ")\n");
