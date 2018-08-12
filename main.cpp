
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <parse_table.h>

#include "./src/paw_print.h"
#include "./src/paw_print_loader.h"


using namespace paw_print;
using namespace parse_table;

using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::to_string;

static void _t_basic () {
    PawPrint pp;

    /*
     * abc:
     *      a: 1
     *      b: 2
     *      c: 3
     *      d: ['i', 'j', 'k', 5, 6, 7]
     *      e:
     *          x: 9.0
     *          y: 8.0
     *          z: 7.0
	 *		f:
	 *			- first:
	 *				name: 'first'
	 *				value: 1
	 *			- second:
	 *				name: 'second'
	 *				value: 2
     */

    pp.beginMap();
        pp.pushKey("abc");
        pp.beginMap();
            pp.pushKey("a"); pp.pushInt(1);
            pp.pushKey("b"); pp.pushInt(2);
            pp.pushKey("c"); pp.pushInt(3);

            pp.pushKey("d");
            pp.beginSequence();
                pp.pushString("i");
                pp.pushString("j");
                pp.pushString("k");
                pp.pushInt(5);
                pp.pushInt(6);
                pp.pushInt(7);
            pp.endSequence();

            pp.pushKey("e");
            pp.beginMap();
                pp.pushKey("x"); pp.pushDouble(9.0);
                pp.pushKey("y"); pp.pushDouble(8.0);
                pp.pushKey("z"); pp.pushDouble(7.0);
            pp.endMap();

			pp.pushKey("f");
			pp.beginSequence();
				pp.beginMap();
					pp.pushKey("first");
					pp.beginMap();
						pp.pushKey("name"); pp.pushString("first");
						pp.pushKey("value"); pp.pushInt(1);
					pp.endMap();
				pp.endMap();
				pp.beginMap();
					pp.pushKey("second");
					pp.beginMap();
						pp.pushKey("name"); pp.pushString("second");
						pp.pushKey("value"); pp.pushInt(2);
					pp.endMap();
				pp.endMap();
			pp.endSequence();

        pp.endMap();
    pp.endMap();

	/*
    std::cout << "size : " << pp.raw_data().size() << std::endl;
    for (auto &c : pp.raw_data())
        std::cout << (int)c << ",";
    std::cout << std::endl;
	exit(0);//*/

	vector<unsigned char> correct_raw_data = { 6,8,3,4,0,97,98,99,0,6,8,3,2,0,97,0,1,1,0,0,0,8,3,2,0,98,0,1,2,0,0,0,8,3,2,0,99,0,1,3,0,0,0,8,3,2,0,100,0,4,3,2,0,105,0,3,2,0,106,0,3,2,0,107,0,1,5,0,0,0,1,6,0,0,0,1,7,0,0,0,5,8,3,2,0,101,0,6,8,3,2,0,120,0,2,0,0,0,0,0,0,34,64,8,3,2,0,121,0,2,0,0,0,0,0,0,32,64,8,3,2,0,122,0,2,0,0,0,0,0,0,28,64,7,8,3,2,0,102,0,4,6,8,3,6,0,102,105,114,115,116,0,6,8,3,5,0,110,97,109,101,0,3,6,0,102,105,114,115,116,0,8,3,6,0,118,97,108,117,101,0,1,1,0,0,0,7,7,6,8,3,7,0,115,101,99,111,110,100,0,6,8,3,5,0,110,97,109,101,0,3,7,0,115,101,99,111,110,100,0,8,3,6,0,118,97,108,117,101,0,1,2,0,0,0,7,7,5,7,7, };
	if (pp.raw_data().size() != correct_raw_data.size()) {
		std::cout << "err: raw_data size error" << std::endl;
		exit(-1);
	}

	for (int ri = 0; ri < correct_raw_data.size(); ++ri) {
		if (pp.raw_data()[ri] != correct_raw_data[ri]) {
			std::cout << "err: raw_data data error" << std::endl;
			exit(-1);
		}
	}

	
    /*
     * abc:
     *      a: 1
     *      b: 2
     *      c: 3
     *      d: ['i', 'j', 'k', 5, 6, 7]
     *      e:
     *          x: 9.0
     *          y: 8.0
     *          z: 7.0
	 *		f:
	 *			- first:
	 *				name: 'first'
	 *				value: 1
	 *			- second:
	 *				name: 'second'
	 *				value: 2
     */
    auto root = pp.root();
	assert(root["xyz"].type() == 0);
	assert(root["abc"].type() == 6);
	assert(root["abc"]["a"].type() == 1);
	assert(root["abc"]["a"].get(-1) == 1);
	assert(root["abc"]["b"].get(-1) == 2);
	assert(root["abc"]["c"].get(-1) == 3);
	assert(root["abc"]["d"].type() == 4);
	assert(strcmp(root["abc"]["d"][0].get(""), "i") == 0);
	assert(strcmp(root["abc"]["d"][1].get(""), "j") == 0);
	assert(strcmp(root["abc"]["d"][2].get(""), "k") == 0);
	assert(strcmp(root["abc"]["d"][3].get(""), "") == 0);
	assert(root["abc"]["d"][3].get(-1.0) == -1.0);
	assert(root["abc"]["d"][3].get(-1) == 5);
	assert(root["abc"]["d"][4].get(-2) == 6);
	assert(root["abc"]["d"][5].get(-3) == 7);
	assert(root["abc"]["e"].type() == 6);
	assert(root["abc"]["e"]["x"].get(-1) == -1);
	assert(root["abc"]["e"]["x"].get(-1.0) == 9.0);
	assert(root["abc"]["e"]["y"].get(-1.0) == 8.0);
	assert(root["abc"]["e"]["z"].get(-1.0) == 7.0);
	assert(root["abc"]["f"].type() == PawPrint::Data::TYPE_SEQUENCE);
	assert(root["abc"]["f"][0].type() == PawPrint::Data::TYPE_MAP);
	assert(root["abc"]["f"][0]["first"].type() == PawPrint::Data::TYPE_MAP);
	assert(root["abc"]["f"][0]["first"]["name"].type() == PawPrint::Data::TYPE_STRING);
	assert(root["abc"]["f"][0]["first"]["value"].type() == PawPrint::Data::TYPE_INT);
	assert(strcmp(root["abc"]["f"][0]["first"]["name"].get(""), "first") == 0);
	assert(root["abc"]["f"][0]["first"]["value"].get(-1) == 1);
	assert(strcmp(root["abc"]["f"][1]["second"]["name"].get(""), "second") == 0);
	assert(root["abc"]["f"][1]["second"]["value"].get(-1) == 2);
}

static shared_ptr<PawPrint> _loadPaw(const string &path) {
	ifstream is(path, std::ifstream::binary);

	// get length of file:
	is.seekg(0, is.end);
	int size = is.tellg();
	is.seekg(0, is.beg);
	// allocate memory:
	char* text = new char[size + 2];
	// read data as a block:
	is.read(text, size);
	text[size] = '\n';
	text[size + 1] = 0;
	is.close();

	auto result = PawPrintLoader::loadText(text);

	delete[] text;

	return result;
}

void _t_load_map_paws() {
	for (int pi = 0; pi <= 5; ++pi) {
		cout << "load_map_paw : " << pi << endl;
#if _WINDOWS
		auto paw = _loadPaw("../../paw/map_0" + to_string(pi) + ".paw");
#else
		auto paw = _loadPaw("../paw/map_0" + to_string(pi) + ".paw");
#endif
		assert(paw != null);
	}
}

static void _t_loadParsingTree () {
#if _WINDOWS
	ifstream f("../../paw_print.tab", std::ifstream::binary);
#else
	ifstream f("paw_print.tab", std::ifstream::binary);
#endif

	// get length of file:
	f.seekg(0, f.end);
	int size = f.tellg();
	f.seekg(0, f.beg);
    
	// allocate memory:
    vector<unsigned char> binary;
    binary.resize(size);

	// read data as a block:
	f.read((char*)binary.data(), size);
	f.close();

    auto table = ParsingTable(binary);
    cout << table.toString();
    auto table_correct = 
		"##### Rules\n" \
		"# Rule 0 : S' -> S \n" \
		"# Rule 1 : S -> NODE \n" \
		"# Rule 2 : KV -> string colon #indent NODE #dedent \n" \
		"# Rule 3 : MAP -> curly_open curly_close \n" \
		"# Rule 4 : MAP -> curly_open MAP_BLOCKED curly_close \n" \
		"# Rule 5 : MAP -> KV MAP \n" \
		"# Rule 6 : MAP -> KV \n" \
		"# Rule 7 : KV_BLOCKED -> string colon NODE \n" \
		"# Rule 8 : MAP_BLOCKED -> KV_BLOCKED \n" \
		"# Rule 9 : MAP_BLOCKED -> KV_BLOCKED comma MAP_BLOCKED \n" \
		"# Rule 10 : SEQ_ELEM -> dash NODE \n" \
		"# Rule 11 : SEQUENCE -> SEQ_ELEM SEQUENCE \n" \
		"# Rule 12 : SEQUENCE -> SEQ_ELEM \n" \
		"# Rule 13 : NODE -> int \n" \
		"# Rule 14 : NODE -> double \n" \
		"# Rule 15 : NODE -> string \n" \
		"# Rule 16 : NODE -> MAP \n" \
		"# Rule 17 : NODE -> SEQUENCE \n" \
		"##### Table\n" \
		"    |   $ | #dedent | #indent | colon | comma | curly_close | curly_open | dash | double | int | string |  KV | KV_BLOCKED |  MAP | MAP_BLOCKED | NODE |    S | SEQUENCE | SEQ_ELEM | \n" \
		"--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
		"  0 |     |         |         |       |       |             |         s5 |   s4 |     s2 |  s1 |     s3 | go6 |            |  go7 |             | go10 | go11 |      go9 |      go8 | \n" \
		"  1 | r13 |     r13 |         |       |   r13 |         r13 |            |  r13 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		"  2 | r14 |     r14 |         |       |   r14 |         r14 |            |  r14 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		"  3 | r15 |     r15 |         |   s12 |   r15 |         r15 |            |  r15 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		"  4 |     |         |         |       |       |             |         s5 |   s4 |     s2 |  s1 |     s3 | go6 |            |  go7 |             | go13 |      |      go9 |      go8 | \n" \
		"  5 |     |         |         |       |       |         s15 |            |      |        |     |    s14 |     |       go16 |      |        go17 |      |      |          |          | \n" \
		"  6 |  r6 |      r6 |         |       |    r6 |          r6 |         s5 |   r6 |        |     |    s18 | go6 |            | go19 |             |      |      |          |          | \n" \
		"  7 | r16 |     r16 |         |       |   r16 |         r16 |            |  r16 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		"  8 | r12 |     r12 |         |       |   r12 |         r12 |            |   s4 |        |     |        |     |            |      |             |      |      |     go20 |      go8 | \n" \
		"  9 | r17 |     r17 |         |       |   r17 |         r17 |            |  r17 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 10 |  r1 |         |         |       |       |             |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 11 | acc |         |         |       |       |             |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 12 |     |         |     s21 |       |       |             |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 13 | r10 |     r10 |         |       |   r10 |         r10 |            |  r10 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 14 |     |         |         |   s22 |       |             |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 15 |  r3 |      r3 |         |       |    r3 |          r3 |            |   r3 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 16 |     |         |         |       |   s23 |          r8 |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 17 |     |         |         |       |       |         s24 |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 18 |     |         |         |   s12 |       |             |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 19 |  r5 |      r5 |         |       |    r5 |          r5 |            |   r5 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 20 | r11 |     r11 |         |       |   r11 |         r11 |            |  r11 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 21 |     |         |         |       |       |             |         s5 |   s4 |     s2 |  s1 |     s3 | go6 |            |  go7 |             | go25 |      |      go9 |      go8 | \n" \
		" 22 |     |         |         |       |       |             |         s5 |   s4 |     s2 |  s1 |     s3 | go6 |            |  go7 |             | go26 |      |      go9 |      go8 | \n" \
		" 23 |     |         |         |       |       |             |            |      |        |     |    s14 |     |       go16 |      |        go27 |      |      |          |          | \n" \
		" 24 |  r4 |      r4 |         |       |    r4 |          r4 |            |   r4 |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 25 |     |     s28 |         |       |       |             |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 26 |     |         |         |       |    r7 |          r7 |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 27 |     |         |         |       |       |          r9 |            |      |        |     |        |     |            |      |             |      |      |          |          | \n" \
		" 28 |  r2 |      r2 |         |       |    r2 |          r2 |         r2 |   r2 |        |     |     r2 |     |            |      |             |      |      |          |          | \n";
    assert(table.toString() == table_correct);

}

int main () {
    _t_basic();
    _t_loadParsingTree();
	_t_load_map_paws();
    return 0;
}
