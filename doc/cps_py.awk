#!/usr/bin/gawk -f
BEGIN { indoc = 0 }
{
    if (index($1, "PyDoc_STRVAR") == 1) {
        if (0 == index($0, ");")) {

            printf ("def ");
            for (ix = 2; ix <= NF; ++ix) {
                pattern = "\"";
                str = $ix;
                res = gsub(pattern, "", str);
                printf ("%s ", str);
            }
            printf (":\n");
            print "    \"\"\""
            indoc = 1;
        }
    }
    else {
        if (1 == indoc) {
            str = $0;
            pattern = "\"";
            gsub(pattern, "", str);
            print str;
            if (0 != index($0, ");")) {
                indoc = 0;
                print "    \"\"\""
            }
        }
    }

}
