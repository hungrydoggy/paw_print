
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

	vector<unsigned char> correct_raw_data = { 7,9,4,4,0,97,98,99,0,7,9,4,2,0,97,0,2,1,0,0,0,9,4,2,0,98,0,2,2,0,0,0,9,4,2,0,99,0,2,3,0,0,0,9,4,2,0,100,0,5,4,2,0,105,0,4,2,0,106,0,4,2,0,107,0,2,5,0,0,0,2,6,0,0,0,2,7,0,0,0,6,9,4,2,0,101,0,7,9,4,2,0,120,0,3,0,0,0,0,0,0,34,64,9,4,2,0,121,0,3,0,0,0,0,0,0,32,64,9,4,2,0,122,0,3,0,0,0,0,0,0,28,64,8,9,4,2,0,102,0,5,7,9,4,6,0,102,105,114,115,116,0,7,9,4,5,0,110,97,109,101,0,4,6,0,102,105,114,115,116,0,9,4,6,0,118,97,108,117,101,0,2,1,0,0,0,8,8,7,9,4,7,0,115,101,99,111,110,100,0,7,9,4,5,0,110,97,109,101,0,4,7,0,115,101,99,111,110,100,0,9,4,6,0,118,97,108,117,101,0,2,2,0,0,0,8,8,6,8,8, };
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
	assert(root["abc"].type() == 7);
	assert(root["abc"]["a"].type() == 2);
	assert(root["abc"]["a"].get(-1) == 1);
	assert(root["abc"]["b"].get(-1) == 2);
	assert(root["abc"]["c"].get(-1) == 3);
	assert(root["abc"]["d"].type() == 5);
	assert(root["abc"]["d"][0].get("") == "i");
	assert(root["abc"]["d"][1].get("") == "j");
	assert(root["abc"]["d"][2].get("") == "k");
	assert(root["abc"]["d"][3].get("") == "5");
	assert(root["abc"]["d"][3].get(-1) == 5);
	assert(root["abc"]["d"][4].get(-2) == 6);
	assert(root["abc"]["d"][5].get(-3) == 7);
	assert(root["abc"]["e"].type() == 7);
	assert(root["abc"]["e"]["x"].get(-1) == -1);
	assert(root["abc"]["e"]["x"].get(-1.0) == 9.0);
	assert(root["abc"]["e"]["y"].get(-1.0) == 8.0);
	assert(root["abc"]["e"]["z"].get(-1.0) == 7.0);
	assert(root["abc"]["f"].type() == PawPrint::Data::TYPE_SEQUENCE);
	assert(root["abc"]["f"][0].type() == PawPrint::Data::TYPE_MAP);
	assert(root["abc"]["f"][0]["first"].type() == PawPrint::Data::TYPE_MAP);
	assert(root["abc"]["f"][0]["first"]["name"].type() == PawPrint::Data::TYPE_STRING);
	assert(root["abc"]["f"][0]["first"]["value"].type() == PawPrint::Data::TYPE_INT);
	assert(root["abc"]["f"][0]["first"]["name"].get("") == "first");
	assert(root["abc"]["f"][0]["first"]["value"].get(-1) == 1);
	assert(root["abc"]["f"][1]["second"]["name"].get("") == "second");
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
    string correct[6] = {
		"a :\n" \
		"  \"abc\"\n",
		"a :\n" \
		"  b :\n" \
		"    \"abc\"\n",
		"a :\n" \
		"  b :\n" \
		"    \"abc\"\n" \
		"  c :\n" \
		"    x :\n" \
		"      1.00000000\n" \
		"    y :\n" \
		"      2.00000000\n" \
		"    z :\n" \
		"      i :\n" \
		"        \"1\"\n" \
		"      j :\n" \
		"        \"2\"\n" \
		"      k :\n" \
		"        \"3\"\n",
		"i :\n" \
		"  \"1\"\n" \
		"j :\n" \
		"  \"2\"\n" \
		"k :\n" \
		"  \"3\"\n",
		"a :\n  ",
		"a :\n" \
		"  b :\n" \
		"    \"abc\"\n" \
		"  c :\n" \
		"    x :\n" \
		"      1.00000000\n" \
		"    y :\n" \
		"      2.00000000\n" \
		"    z :\n" \
		"      i :\n" \
		"        \"1\"\n" \
		"      j :\n" \
		"        \"2\"\n" \
		"      k :\n" \
		"        \"3\"\n" \
		"  d :\n" \
		"    13\n",
    };

	for (int pi = 0; pi <= 5; ++pi) {
	//for (int pi = 2; pi <= 2; ++pi) {
		cout << "load_map_paw : " << pi << endl;
#if _WINDOWS
		auto paw = _loadPaw("../../paw/map_0" + to_string(pi) + ".paw");
#else
		auto paw = _loadPaw("../paw/map_0" + to_string(pi) + ".paw");
#endif
		assert(paw != null);

        assert(paw->root().toString() == correct[pi]);
	}
}

static void _t_loadParsingTree () {
#if _WINDOWS
	ifstream f("../../paw_print.tab", std::ifstream::binary);
#else
	ifstream f("../paw_print.tab", std::ifstream::binary);
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
    //cout << table.toString();
    auto table_correct = 
		"##### Rules\n" \
		"# Rule 0 : S' -> S \n" \
		"# Rule 1 : S -> NODE \n" \
		"# Rule 2 : KV -> string colon #indent NODE #dedent \n" \
		"# Rule 3 : KV -> string colon NODE \n" \
		"# Rule 4 : MAP -> curly_open curly_close \n" \
		"# Rule 5 : MAP -> curly_open MAP_BLOCKED curly_close \n" \
		"# Rule 6 : MAP -> KV MAP \n" \
		"# Rule 7 : MAP -> KV \n" \
		"# Rule 8 : KV_BLOCKED -> string colon NODE \n" \
		"# Rule 9 : MAP_BLOCKED -> KV_BLOCKED \n" \
		"# Rule 10 : MAP_BLOCKED -> KV_BLOCKED comma MAP_BLOCKED \n" \
		"# Rule 11 : SEQ_ELEM -> dash NODE \n" \
		"# Rule 12 : SEQ_ELEM -> dash #indent NODE #dedent \n" \
		"# Rule 13 : SEQUENCE -> square_open square_close \n" \
		"# Rule 14 : SEQUENCE -> square_open SEQ_BLOCKED square_close \n" \
		"# Rule 15 : SEQUENCE -> SEQ_ELEM SEQUENCE \n" \
		"# Rule 16 : SEQUENCE -> SEQ_ELEM \n" \
		"# Rule 17 : SEQ_BLOCKED -> NODE comma SEQ_BLOCKED \n" \
		"# Rule 18 : SEQ_BLOCKED -> NODE \n" \
		"# Rule 19 : NODE -> int \n" \
		"# Rule 20 : NODE -> double \n" \
		"# Rule 21 : NODE -> string \n" \
		"# Rule 22 : NODE -> MAP \n" \
		"# Rule 23 : NODE -> SEQUENCE \n" \
		"##### Table\n" \
		"    |   $ | #dedent | #indent | colon | comma | curly_close | curly_open | dash | double | int | square_close | square_open | string |  KV | KV_BLOCKED |  MAP | MAP_BLOCKED | NODE |   S | SEQUENCE | SEQ_BLOCKED | SEQ_ELEM | \n" \
		"--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
		"  0 |     |         |         |       |       |             |         s2 |  s12 |     s5 |  s4 |              |          s6 |     s3 | go1 |            |  go7 |             | go11 | go9 |     go10 |             |      go8 | \n" \
		"  1 |  r7 |      r7 |         |       |    r7 |          r7 |         s2 |   r7 |        |     |           r7 |          r7 |    s13 | go1 |            | go14 |             |      |     |          |             |          | \n" \
		"  2 |     |         |         |       |       |         s16 |            |      |        |     |              |             |    s15 |     |       go17 |      |        go18 |      |     |          |             |          | \n" \
		"  3 | r21 |     r21 |         |   s19 |   r21 |         r21 |        r21 |  r21 |        |     |          r21 |         r21 |    r21 |     |            |      |             |      |     |          |             |          | \n" \
		"  4 | r19 |     r19 |         |       |   r19 |         r19 |        r19 |  r19 |        |     |          r19 |         r19 |    r19 |     |            |      |             |      |     |          |             |          | \n" \
		"  5 | r20 |     r20 |         |       |   r20 |         r20 |        r20 |  r20 |        |     |          r20 |         r20 |    r20 |     |            |      |             |      |     |          |             |          | \n" \
		"  6 |     |         |         |       |       |             |         s2 |  s12 |     s5 |  s4 |          s20 |          s6 |     s3 | go1 |            |  go7 |             | go22 |     |     go10 |        go21 |      go8 | \n" \
		"  7 | r22 |     r22 |         |       |   r22 |         r22 |        r22 |  r22 |        |     |          r22 |         r22 |    r22 |     |            |      |             |      |     |          |             |          | \n" \
		"  8 | r16 |     r16 |         |       |   r16 |         r16 |        r16 |  s12 |        |     |          r16 |          s6 |    r16 |     |            |      |             |      |     |     go23 |             |      go8 | \n" \
		"  9 | acc |         |         |       |       |             |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 10 | r23 |     r23 |         |       |   r23 |         r23 |        r23 |  r23 |        |     |          r23 |         r23 |    r23 |     |            |      |             |      |     |          |             |          | \n" \
		" 11 |  r1 |         |         |       |       |             |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 12 |     |         |     s24 |       |       |             |         s2 |  s12 |     s5 |  s4 |              |          s6 |     s3 | go1 |            |  go7 |             | go25 |     |     go10 |             |      go8 | \n" \
		" 13 |     |         |         |   s19 |       |             |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 14 |  r6 |      r6 |         |       |    r6 |          r6 |         r6 |   r6 |        |     |           r6 |          r6 |     r6 |     |            |      |             |      |     |          |             |          | \n" \
		" 15 |     |         |         |   s26 |       |             |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 16 |  r4 |      r4 |         |       |    r4 |          r4 |         r4 |   r4 |        |     |           r4 |          r4 |     r4 |     |            |      |             |      |     |          |             |          | \n" \
		" 17 |     |         |         |       |   s27 |          r9 |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 18 |     |         |         |       |       |         s28 |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 19 |     |         |     s29 |       |       |             |         s2 |  s12 |     s5 |  s4 |              |          s6 |     s3 | go1 |            |  go7 |             | go30 |     |     go10 |             |      go8 | \n" \
		" 20 | r13 |     r13 |         |       |   r13 |         r13 |        r13 |  r13 |        |     |          r13 |         r13 |    r13 |     |            |      |             |      |     |          |             |          | \n" \
		" 21 |     |         |         |       |       |             |            |      |        |     |          s31 |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 22 |     |         |         |       |   s32 |             |            |      |        |     |          r18 |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 23 | r15 |     r15 |         |       |   r15 |         r15 |        r15 |  r15 |        |     |          r15 |         r15 |    r15 |     |            |      |             |      |     |          |             |          | \n" \
		" 24 |     |         |         |       |       |             |         s2 |  s12 |     s5 |  s4 |              |          s6 |     s3 | go1 |            |  go7 |             | go33 |     |     go10 |             |      go8 | \n" \
		" 25 | r11 |     r11 |         |       |   r11 |         r11 |        r11 |  r11 |        |     |          r11 |         r11 |    r11 |     |            |      |             |      |     |          |             |          | \n" \
		" 26 |     |         |         |       |       |             |         s2 |  s12 |     s5 |  s4 |              |          s6 |     s3 | go1 |            |  go7 |             | go34 |     |     go10 |             |      go8 | \n" \
		" 27 |     |         |         |       |       |             |            |      |        |     |              |             |    s15 |     |       go17 |      |        go35 |      |     |          |             |          | \n" \
		" 28 |  r5 |      r5 |         |       |    r5 |          r5 |         r5 |   r5 |        |     |           r5 |          r5 |     r5 |     |            |      |             |      |     |          |             |          | \n" \
		" 29 |     |         |         |       |       |             |         s2 |  s12 |     s5 |  s4 |              |          s6 |     s3 | go1 |            |  go7 |             | go36 |     |     go10 |             |      go8 | \n" \
		" 30 |  r3 |      r3 |         |       |    r3 |          r3 |         r3 |   r3 |        |     |           r3 |          r3 |     r3 |     |            |      |             |      |     |          |             |          | \n" \
		" 31 | r14 |     r14 |         |       |   r14 |         r14 |        r14 |  r14 |        |     |          r14 |         r14 |    r14 |     |            |      |             |      |     |          |             |          | \n" \
		" 32 |     |         |         |       |       |             |         s2 |  s12 |     s5 |  s4 |              |          s6 |     s3 | go1 |            |  go7 |             | go22 |     |     go10 |        go37 |      go8 | \n" \
		" 33 |     |     s38 |         |       |       |             |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 34 |     |         |         |       |    r8 |          r8 |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 35 |     |         |         |       |       |         r10 |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 36 |     |     s39 |         |       |       |             |            |      |        |     |              |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 37 |     |         |         |       |       |             |            |      |        |     |          r17 |             |        |     |            |      |             |      |     |          |             |          | \n" \
		" 38 | r12 |     r12 |         |       |   r12 |         r12 |        r12 |  r12 |        |     |          r12 |         r12 |    r12 |     |            |      |             |      |     |          |             |          | \n" \
		" 39 |  r2 |      r2 |         |       |    r2 |          r2 |         r2 |   r2 |        |     |           r2 |          r2 |     r2 |     |            |      |             |      |     |          |             |          | \n";
    assert(table.toString() == table_correct);

}

static void _t_load_boss_appear_snake () {
    cout << "load_boss_appear_snake.obj" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/boss_appear_snake.obj");
#else
    auto paw = _loadPaw("../paw/boss_appear_snake.obj");
#endif
    assert(paw != null);

    auto correct =
		"@objects/ui/popup/boss_appear.obj :\n" \
		"  ref :\n" \
		"    boss_img :\n" \
		"      \"images/ui/popup/snake_art.png\"\n" \
		"    icon_img :\n" \
		"      \"images/status/icons/inven_ico_snake_poison.png\"\n" \
		"    icon_pos :\n" \
		"      - 0\n" \
		"      - 10.50000000\n" \
		"      - 1\n" \
		"    icon_siz :\n" \
		"      - 79\n" \
		"      - 0\n";
    assert(paw->root().toString() == correct);
}

static void _t_load_green () {
    cout << "load green.obj" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/green.obj");
#else
    auto paw = _loadPaw("../paw/green.obj");
#endif
    assert(paw != null);

    auto correct =
		"Object :\n" \
		"  chl :\n" \
		"    - Object :\n" \
		"        dra :\n" \
		"          \"false\"\n" \
		"        nam :\n" \
		"          \"bg_gray\"\n" \
		"        vie :\n" \
		"          NinePatch :\n" \
		"            img :\n" \
		"              \"images/ui/button/gray.png\"\n" \
		"            lrt :\n" \
		"              - 10\n" \
		"              - 10\n" \
		"              - 10\n" \
		"              - 10\n" \
		"            siz :\n" \
		"              - $width :\n" \
		"                  400\n" \
		"              - $height :\n" \
		"                  80\n" \
		"    - Object :\n" \
		"        bdy :\n" \
		"          BulletBody :\n" \
		"            mas :\n" \
		"              0\n" \
		"        nam :\n" \
		"          \"bg\"\n" \
		"        pda :\n" \
		"          ActionCall :\n" \
		"            act :\n" \
		"              SendMessage :\n" \
		"                dst :\n" \
		"                  $pda_dst :\n" \
		"                    \"/\"\n" \
		"                nam :\n" \
		"                  $pda_msg :\n" \
		"                    \"onButtonClicked\"\n" \
		"                par :\n" \
		"                  $pda_par :\n" \
		"                                roo :\n" \
		"              \"..\"\n" \
		"        shp :\n" \
		"          - BulletShape :\n" \
		"              cls :\n" \
		"                \"Box\"\n" \
		"              hsz :\n" \
		"                - !divide :\n" \
		"                    - $width :\n" \
		"                        400\n" \
		"                    - 2\n" \
		"                - !divide :\n" \
		"                    - $height :\n" \
		"                        80\n" \
		"                    - 2\n" \
		"                - 0\n" \
		"        vie :\n" \
		"          NinePatch :\n" \
		"            img :\n" \
		"              \"images/ui/button/green.png\"\n" \
		"            lrt :\n" \
		"              - 10\n" \
		"              - 10\n" \
		"              - 10\n" \
		"              - 10\n" \
		"            siz :\n" \
		"              - $width :\n" \
		"                  400\n" \
		"              - $height :\n" \
		"                  80\n" \
		"    - Object :\n" \
		"        nam :\n" \
		"          \"text\"\n" \
		"        pos :\n" \
		"          $text_pos :\n" \
		"            - 0\n" \
		"            - 0\n" \
		"            - 1\n" \
		"        scl :\n" \
		"          $text_scl :\n" \
		"            - 0.70000000\n" \
		"            - 0.70000000\n" \
		"            - 1\n" \
		"        vie :\n" \
		"          BitmapFontView :\n" \
		"            bmf :\n" \
		"              $font :\n" \
		"                \"bitmap_fonts/nanum_myeongjo_extra_bold.67.bmf\"\n" \
		"            hor :\n" \
		"              \"middle\"\n" \
		"            txt :\n" \
		"              $text :\n" \
		"                \"확 인\"\n" \
		"            ver :\n" \
		"              \"middle\"\n" \
		"  cls :\n" \
		"    \"Button\"\n" \
		"  col :\n" \
		"    $button_col :\n" \
		"      - 1\n" \
		"      - 1\n" \
		"      - 1\n" \
		"      - 1\n" \
		"  nam :\n" \
		"    \"button\"\n";
    assert(paw->root().toString() == correct);
}

int main () {
    _t_basic();
    _t_loadParsingTree();
	_t_load_map_paws();
	_t_load_boss_appear_snake();
	_t_load_green();
    return 0;
}
