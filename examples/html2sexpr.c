/*
 * html2sexpr
 * Convert html tag tree into s-expression string in stdout
  */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <myhtml/api.h>

#define DIE(msg, ...) do { fprintf(stderr, msg, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0);

static const char* tag_str(myhtml_tag_id_t id)
{
    switch (id)
    {
    case MyTAGS_TAG__UNDEF:        return "_UNDEF";
    case MyTAGS_TAG__TEXT:         return "_TEXT";
    case MyTAGS_TAG__COMMENT:      return "_COMMENT";   
    case MyTAGS_TAG__DOCTYPE:      return "_DOCTYPE";
    case MyTAGS_TAG_A:             return "A";
    case MyTAGS_TAG_ABBR:          return "ABBR";
    case MyTAGS_TAG_ACRONYM:       return "ACRONYM";
    case MyTAGS_TAG_ADDRESS:       return "ADDRESS";
    case MyTAGS_TAG_ANNOTATION_XML:return "ANNOTATION_XML";
    case MyTAGS_TAG_APPLET:        return "APPLET";
    case MyTAGS_TAG_AREA:          return "AREA";
    case MyTAGS_TAG_ARTICLE:       return "ARTICLE";
    case MyTAGS_TAG_ASIDE:         return "ASIDE";
    case MyTAGS_TAG_AUDIO:         return "AUDIO";
    case MyTAGS_TAG_B:             return "B";
    case MyTAGS_TAG_BASE:          return "BASE";
    case MyTAGS_TAG_BASEFONT:      return "BASEFONT";
    case MyTAGS_TAG_BDI:           return "BDI";
    case MyTAGS_TAG_BDO:           return "BDO";
    case MyTAGS_TAG_BGSOUND:       return "BGSOUND";
    case MyTAGS_TAG_BIG:           return "BIG";
    case MyTAGS_TAG_BLINK:         return "BLINK";
    case MyTAGS_TAG_BLOCKQUOTE:    return "BLOCKQUOTE";
    case MyTAGS_TAG_BODY:          return "BODY";
    case MyTAGS_TAG_BR:            return "BR";
    case MyTAGS_TAG_BUTTON:        return "BUTTON";
    case MyTAGS_TAG_CANVAS:        return "CANVAS";
    case MyTAGS_TAG_CAPTION:       return "CAPTION";
    case MyTAGS_TAG_CENTER:        return "CENTER";
    case MyTAGS_TAG_CITE:          return "CITE";
    case MyTAGS_TAG_CODE:          return "CODE";
    case MyTAGS_TAG_COL:           return "COL";
    case MyTAGS_TAG_COLGROUP:      return "COLGROUP";
    case MyTAGS_TAG_COMMAND:       return "COMMAND";
    case MyTAGS_TAG_COMMENT:       return "COMMENT";
    case MyTAGS_TAG_DATALIST:      return "DATALIST";
    case MyTAGS_TAG_DD:            return "DD";
    case MyTAGS_TAG_DEL:           return "DEL";
    case MyTAGS_TAG_DESC:          return "DESC";
    case MyTAGS_TAG_DETAILS:       return "DETAILS";
    case MyTAGS_TAG_DFN:           return "DFN";
    case MyTAGS_TAG_DIALOG:        return "DIALOG";
    case MyTAGS_TAG_DIR:           return "DIR";
    case MyTAGS_TAG_DIV:           return "DIV";
    case MyTAGS_TAG_DL:            return "DL";
    case MyTAGS_TAG_DT:            return "DT";
    case MyTAGS_TAG_EM:            return "EM";
    case MyTAGS_TAG_EMBED:         return "EMBED";
    case MyTAGS_TAG_FIELDSET:      return "FIELDSET";
    case MyTAGS_TAG_FIGCAPTION:    return "FIGCAPTION";
    case MyTAGS_TAG_FIGURE:        return "FIGURE";
    case MyTAGS_TAG_FONT:          return "FONT";
    case MyTAGS_TAG_FOOTER:        return "FOOTER";
    case MyTAGS_TAG_FOREIGNOBJECT: return "FOREIGNOBJECT";
    case MyTAGS_TAG_FORM:          return "FORM";
    case MyTAGS_TAG_FRAME:         return "FRAME";
    case MyTAGS_TAG_FRAMESET:      return "FRAMESET";
    case MyTAGS_TAG_H1:            return "H1";
    case MyTAGS_TAG_H2:            return "H2";
    case MyTAGS_TAG_H3:            return "H3";
    case MyTAGS_TAG_H4:            return "H4";
    case MyTAGS_TAG_H5:            return "H5";
    case MyTAGS_TAG_H6:            return "H6";
    case MyTAGS_TAG_HEAD:          return "HEAD";
    case MyTAGS_TAG_HEADER:        return "HEADER";
    case MyTAGS_TAG_HGROUP:        return "HGROUP";
    case MyTAGS_TAG_HR:            return "HR";
    case MyTAGS_TAG_HTML:          return "HTML";
    case MyTAGS_TAG_I:             return "I";
    case MyTAGS_TAG_IFRAME:        return "IFRAME";
    case MyTAGS_TAG_IMAGE:         return "IMAGE";
    case MyTAGS_TAG_IMG:           return "IMG";
    case MyTAGS_TAG_INPUT:         return "INPUT";
    case MyTAGS_TAG_INS:           return "INS";
    case MyTAGS_TAG_ISINDEX:       return "ISINDEX";
    case MyTAGS_TAG_KBD:           return "KBD";
    case MyTAGS_TAG_KEYGEN:        return "KEYGEN";
    case MyTAGS_TAG_LABEL:         return "LABEL";
    case MyTAGS_TAG_LEGEND:        return "LEGEND";
    case MyTAGS_TAG_LI:            return "LI";
    case MyTAGS_TAG_LINK:          return "LINK";
    case MyTAGS_TAG_LISTING:       return "LISTING";
    case MyTAGS_TAG_MAIN:          return "MAIN";
    case MyTAGS_TAG_MALIGNMARK:    return "MALIGNMARK";
    case MyTAGS_TAG_MAP:           return "MAP";
    case MyTAGS_TAG_MARK:          return "MARK";
    case MyTAGS_TAG_MARQUEE:       return "MARQUEE";
    case MyTAGS_TAG_MATH:          return "MATH";
    case MyTAGS_TAG_MENU:          return "MENU";
    case MyTAGS_TAG_MENUITEM:      return "MENUITEM";
    case MyTAGS_TAG_MERROR:        return "MERROR";
    case MyTAGS_TAG_META:          return "META";
    case MyTAGS_TAG_METER:         return "METER";
    case MyTAGS_TAG_MGLYPH:        return "MGLYPH";
    case MyTAGS_TAG_MI:            return "MI";
    case MyTAGS_TAG_MO:            return "MO";
    case MyTAGS_TAG_MN:            return "MN";
    case MyTAGS_TAG_MS:            return "MS";
    case MyTAGS_TAG_MTEXT:         return "MTEXT";
    case MyTAGS_TAG_NAV:           return "NAV";
    case MyTAGS_TAG_NOBR:          return "NOBR";
    case MyTAGS_TAG_NOEMBED:       return "NOEMBED";
    case MyTAGS_TAG_NOFRAMES:      return "NOFRAMES";
    case MyTAGS_TAG_NOSCRIPT:      return "NOSCRIPT";
    case MyTAGS_TAG_OBJECT:        return "OBJECT";
    case MyTAGS_TAG_OL:            return "OL";
    case MyTAGS_TAG_OPTGROUP:      return "OPTGROUP";
    case MyTAGS_TAG_OPTION:        return "OPTION";
    case MyTAGS_TAG_OUTPUT:        return "OUTPUT";
    case MyTAGS_TAG_P:             return "P";
    case MyTAGS_TAG_PARAM:         return "PARAM";
    case MyTAGS_TAG_PLAINTEXT:     return "PLAINTEXT";
    case MyTAGS_TAG_PRE:           return "PRE";
    case MyTAGS_TAG_PROGRESS:      return "PROGRESS";
    case MyTAGS_TAG_Q:             return "Q";
    case MyTAGS_TAG_RB:            return "RB";
    case MyTAGS_TAG_RP:            return "RP";
    case MyTAGS_TAG_RT:            return "RT";
    case MyTAGS_TAG_RTC:           return "RTC";
    case MyTAGS_TAG_RUBY:          return "RUBY";
    case MyTAGS_TAG_S:             return "S";
    case MyTAGS_TAG_SAMP:          return "SAMP";
    case MyTAGS_TAG_SCRIPT:        return "SCRIPT";
    case MyTAGS_TAG_SECTION:       return "SECTION";
    case MyTAGS_TAG_SELECT:        return "SELECT";
    case MyTAGS_TAG_SMALL:         return "SMALL";
    case MyTAGS_TAG_SOURCE:        return "SOURCE";
    case MyTAGS_TAG_SPAN:          return "SPAN";
    case MyTAGS_TAG_STRIKE:        return "STRIKE";
    case MyTAGS_TAG_STRONG:        return "STRONG";
    case MyTAGS_TAG_STYLE:         return "STYLE";
    case MyTAGS_TAG_SUB:           return "SUB";
    case MyTAGS_TAG_SUMMARY:       return "SUMMARY";
    case MyTAGS_TAG_SUP:           return "SUP";
    case MyTAGS_TAG_SVG:           return "SVG";
    case MyTAGS_TAG_TABLE:         return "TABLE";
    case MyTAGS_TAG_TBODY:         return "TBODY";
    case MyTAGS_TAG_TD:            return "TD";
    case MyTAGS_TAG_TEMPLATE:      return "TEMPLATE";
    case MyTAGS_TAG_TEXTAREA:      return "TEXTAREA";
    case MyTAGS_TAG_TFOOT:         return "TFOOT";
    case MyTAGS_TAG_TH:            return "TH";
    case MyTAGS_TAG_THEAD:         return "THEAD";
    case MyTAGS_TAG_TIME:          return "TIME";
    case MyTAGS_TAG_TITLE:         return "TITLE";
    case MyTAGS_TAG_TR:            return "TR";
    case MyTAGS_TAG_TRACK:         return "TRACK";
    case MyTAGS_TAG_TT:            return "TT";
    case MyTAGS_TAG_U:             return "U";
    case MyTAGS_TAG_UL:            return "UL";
    case MyTAGS_TAG_VAR:           return "VAR";
    case MyTAGS_TAG_VIDEO:         return "VIDEO";
    case MyTAGS_TAG_WBR:           return "WBR";
    case MyTAGS_TAG_XMP:           return "XMP";
    case MyTAGS_TAG__END_OF_FILE:  return "_END_OF_FILE";
    default:                       assert(0); return "(unknown)";
    };
}

static bool filter_node(myhtml_tree_node_t* node) 
{
    assert(node);
    myhtml_tag_id_t tag = myhtml_node_tag_id(node);
    return (tag != MyTAGS_TAG__TEXT) && (tag != MyTAGS_TAG__END_OF_FILE) && (tag != MyTAGS_TAG__COMMENT) && (tag != MyTAGS_TAG__UNDEF);
}

/* depth-first lefthand tree walk */
static void walk_subtree(myhtml_tree_node_t* root, int level)
{
    if (!root) {
        return;
    }

    /* Check if we handle this node type */
    if (!filter_node(root)) {
        return;
    }

    /* start sexpr */
    putchar('(');

    /* print this node */
    printf("%s", tag_str(myhtml_node_tag_id(root)));
    myhtml_tree_attr_t* attr = myhtml_node_attribute_first(root);
    while (attr != NULL) {
        /* attribute sexpr (name value)*/
        printf("(%s \'%s\')", myhtml_attribute_name(attr, NULL), myhtml_attribute_value(attr, NULL));
        attr = myhtml_attribute_next(attr);
    }

    /* left hand depth-first recoursion */
    myhtml_tree_node_t* child = myhtml_node_child(root);
    if (child != NULL) {
        while (child != NULL) {
            
            walk_subtree(child, level + 1);
            child = myhtml_node_next(child);
        }
    }

    /* close sexpr */
    putchar(')');
}

struct res_html {
    char  *html;
    size_t size;
};

struct res_html load_html_file(const char* filename)
{
    FILE *fh = fopen(filename, "rb");
    if(fh == NULL) {
        fprintf(stderr, "Can't open html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    fseek(fh, 0L, SEEK_END);
    long size = ftell(fh);
    fseek(fh, 0L, SEEK_SET);
    
    char *html = (char*)malloc(size + 1);
    if(html == NULL) {
        DIE("Can't allocate mem for html file: %s\n", filename);
    }
    
    size_t nread = fread(html, 1, size, fh);
    if (nread != size) {
        DIE("could not read %ld bytes (%zu bytes done)\n", size, nread);
    }

    fclose(fh);
    
    if(size < 0) {
        size = 0;
    }
    
    struct res_html res = {html, (size_t)size};
    return res;
}

static void usage(void)
{
    fprintf(stderr, "html2sexpr <file>\n");
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		usage();
		DIE("Invalid number of arguments\n");
	}

	struct res_html data = load_html_file(argv[1]);
	myhtml_status_t res = MyHTML_STATUS_OK;

	// basic init
    myhtml_t* myhtml = myhtml_create();
    res = myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    if (MYHTML_FAILED(res)) {
    	DIE("myhtml_init failed with %d\n", res);
    }
    
    // init tree
    // XXX: Why do we need to explicitly create a tree before calling parse_html? Can't parse_html just create and return a new tree for us?
    myhtml_tree_t* tree = myhtml_tree_create();
    res = myhtml_tree_init(tree, myhtml);
    if (MYHTML_FAILED(res)) {
    	DIE("myhtml_tree_init failed with %d\n", res);
    }
    
    // parse html
    myhtml_parse(tree, data.html, data.size);
    
    // Find <html>, we'll dump from there to make things easier
    myhtml_collection_t* collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyTAGS_TAG_HTML, &res);
    if (!collection) {
        DIE("Can't find root node: %d\n", res);
    }

    walk_subtree(collection->list[0], 0);
    printf("\n");

    // release resources
    myhtml_collection_destroy(collection);
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    free(data.html);
	return EXIT_SUCCESS;
}