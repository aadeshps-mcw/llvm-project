// RUN: %clang_cc1 -fsyntax-only -verify %s -DNONE -Wno-gnu
// RUN: %clang_cc1 -fsyntax-only -verify %s -DALL -Wgnu
// RUN: %clang_cc1 -fsyntax-only -verify %s -DALL -Wno-gnu \
// RUN:   -Wgnu-alignof-expression -Wgnu-complex-integer -Wgnu-conditional-omitted-operand \
// RUN:   -Wgnu-label-as-value -Wgnu-statement-expression \
// RUN:   -Wgnu-compound-literal-initializer -Wgnu-flexible-array-initializer \
// RUN:   -Wgnu-redeclared-enum  -Wgnu-folding-constant -Wgnu-empty-struct \
// RUN:   -Wgnu-union-cast -Wgnu-variable-sized-type-not-at-end
// RUN: %clang_cc1 -fsyntax-only -verify %s -DNONE -Wgnu \
// RUN:   -Wno-gnu-alignof-expression -Wno-gnu-complex-integer -Wno-gnu-conditional-omitted-operand \
// RUN:   -Wno-gnu-label-as-value -Wno-gnu-statement-expression \
// RUN:   -Wno-gnu-compound-literal-initializer -Wno-gnu-flexible-array-initializer \
// RUN:   -Wno-gnu-redeclared-enum -Wno-gnu-folding-constant -Wno-gnu-empty-struct \
// RUN:   -Wno-gnu-union-cast -Wno-gnu-variable-sized-type-not-at-end
// Additional disabled tests:
// %clang_cc1 -fsyntax-only -verify %s -DALIGNOF -Wno-gnu -Wgnu-alignof-expression
// %clang_cc1 -fsyntax-only -verify %s -DCOMPLEXINT -Wno-gnu -Wgnu-complex-integer
// %clang_cc1 -fsyntax-only -verify %s -DOMITTEDOPERAND -Wno-gnu -Wgnu-conditional-omitted-operand
// %clang_cc1 -fsyntax-only -verify %s -DLABELVALUE -Wno-gnu -Wgnu-label-as-value
// %clang_cc1 -fsyntax-only -verify %s -DSTATEMENTEXP -Wno-gnu -Wgnu-statement-expression
// %clang_cc1 -fsyntax-only -verify %s -DSTATEMENTEXPMACRO -Wno-gnu -Wgnu-statement-expression-from-macro-expansion
// %clang_cc1 -fsyntax-only -verify %s -DCOMPOUNDLITERALINITIALIZER -Wno-gnu -Wgnu-compound-literal-initializer
// %clang_cc1 -fsyntax-only -verify %s -DFLEXIBLEARRAYINITIALIZER -Wno-gnu -Wgnu-flexible-array-initializer
// %clang_cc1 -fsyntax-only -verify %s -DREDECLAREDENUM -Wno-gnu -Wgnu-redeclared-enum
// %clang_cc1 -fsyntax-only -verify %s -DUNIONCAST -Wno-gnu -Wgnu-union-cast
// %clang_cc1 -fsyntax-only -verify %s -DVARIABLESIZEDTYPENOTATEND -Wno-gnu -Wgnu-variable-sized-type-not-at-end
// %clang_cc1 -fsyntax-only -verify %s -DFOLDINGCONSTANT -Wno-gnu -Wgnu-folding-constant
// %clang_cc1 -fsyntax-only -verify %s -DEMPTYSTRUCT -Wno-gnu -Wgnu-empty-struct

#if NONE
// expected-no-diagnostics
#endif


#if ALL || ALIGNOF
// expected-warning@+4 {{'_Alignof' applied to an expression is a GNU extension}}
#endif

char align;
_Static_assert(_Alignof(align) > 0, "align's alignment is wrong");


#if ALL || COMPLEXINT
// expected-warning@+3 {{complex integer types are a GNU extension}}
#endif

_Complex short int complexint;


#if ALL || OMITTEDOPERAND
// expected-warning@+3 {{use of GNU ?: conditional expression extension, omitting middle operand}}
#endif

static const char* omittedoperand = (const char*)0 ?: "Null";


#if ALL || LABELVALUE
// expected-warning@+6 {{use of GNU address-of-label extension}}
// expected-warning@+7 {{use of GNU indirect-goto extension}}
#endif

void labelvalue(void) {
	void *ptr;
	ptr = &&foo;
foo:
	goto *ptr;
}


#if ALL || STATEMENTEXP
// expected-warning@+5 {{use of GNU statement expression extension}}
#endif

void statementexp(void)
{
	int a = ({ 1; });
}

#if ALL || STATEMENTEXP || STATEMENTEXPMACRO
// expected-warning@+5 {{use of GNU statement expression extension from macro expansion}}
#endif

#define STMT_EXPR_MACRO(a) ({ (a); })
void statementexprmacro(void) {
  int a = STMT_EXPR_MACRO(1);
}

#if ALL || COMPOUNDLITERALINITIALIZER
// expected-warning@+4 {{initialization of an array of type 'int[5]' from a compound literal of type 'int[5]' is a GNU extension}}
#endif

typedef int int5[5];
int cli[5] = (int[]){1, 2, 3, 4, 5};


#if ALL || FLEXIBLEARRAYINITIALIZER
// expected-note@+6 {{initialized flexible array member 'y' is here}}
// expected-warning@+6 {{flexible array initialization is a GNU extension}}
#endif

struct fai {
  int x;
  int y[];
} fai = { 1, { 2, 3, 4 } };


#if ALL || FOLDINGCONSTANT
// expected-warning@+5 {{expression is not an integer constant expression; folding it to a constant is a GNU extension}}
// expected-warning@+7 {{variable length array folded to constant array as an extension}}
#endif

enum {
	fic = (int)(0.75 * 1000 * 1000)
};
static const int size = 100;
int data[size];

void foo(void) { int data[size]; } // OK, always a VLA

#if ALL || REDECLAREDENUM
// expected-note@+4 {{previous definition is here}}
// expected-warning@+8 {{redeclaration of already-defined enum 'RE' is a GNU extension}}
#endif

enum RE {
  Val1,
  Val2
};

enum RE;


#if ALL || UNIONCAST
// expected-warning@+4 {{cast to union type is a GNU extension}}
#endif

union uc { int i; unsigned : 3; };
union uc w = (union uc)2;


#if ALL || VARIABLESIZEDTYPENOTATEND
// expected-warning@+8 {{field 'hdr' with variable sized type 'struct vst' not at the end of a struct or class is a GNU extension}}
#endif

struct vst {
 short tag_type;
 char tag_data[];
};
struct vstnae {
  struct vst hdr;
  char data;
};


#if ALL || EMPTYSTRUCT
// expected-warning@+4 {{empty struct is a GNU extension}}
// expected-warning@+4 {{struct without named members is a GNU extension}}
#endif

const struct {} es;
struct {int:5;} swnm;

