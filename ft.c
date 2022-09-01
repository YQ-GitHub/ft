/* 
 * ft.c -- a family tree system.
 * Copyright (C) 2022-2022
 * Created by cnluyq @2022 
 * This program is distributed in the hope that it will be useful,but without any warranty.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ulong unsigned long
#define uint unsigned int
#define ushort unsigned short
#define NAME_LEN 128
#define DATE_LEN 128
#define PATH_LEN 1024
#define BUFF_LEN 3072
#define NOTE_LEN 1024 
#define LINK_LEN 1024
#define ID_LEN 16
#define MAX_DUPNAME 1024
#define MAX_GEN 1024
#define BUFF_PAD 128
#define MAX_KEY 16
#define MIN_KEY 6 
#define CRCLEN 4

enum type {
   ALL_TYPE,
   PERSON_TYPE,
   PERSONINLAW_TYPE,
};

enum movedirection {
   UP,
   DOWN,
};

enum printtype {
    PRINTALL,
    PRINTNAMENOTE,
    PRINTNAMESEX,
    PRINTNAMEONLY,
};

typedef struct PersonInLaw {
  ulong id;
  char name[NAME_LEN];
  ushort sex;
  struct Person * relation;
  char note[NOTE_LEN];
  char link[LINK_LEN];
}PersonInLaw_t;

typedef struct Person {
  ulong id; 
  char name[NAME_LEN];
  ushort sex;
  ulong generation;
  ulong relative_generation;
  ushort n_childreninclan;
  ushort n_childreninclan_print;
  ushort order; 
  struct Person ** childreninclan;
  ushort n_spouse;
  struct PersonInLaw ** spouse;
  struct Person * father;
  struct Person * mother;
  char note[NOTE_LEN];
  char up_ft[NAME_LEN];
  char down_ft[NAME_LEN];
  char link[LINK_LEN];
}Person_t;

ulong g_id=1; 
Person_t * g_tree_root=NULL;
char ftn[NAME_LEN]={0};
char ft_note[NOTE_LEN]={0};

char *msg_zh[]={
"开始创建族谱，请给族谱起个名字(不能包含空格等特殊字符)",  //0
"请输入初祖的名字", //1
"男", //2
"女", //3
"已经存在，请重试", //4
"世代数", //5
"性别", //6
"排行", //7
"孩子数", //8
"参数为空", //9
"孩子分别是", //10
"配偶", //11
"是否有重名", //12
"是", //13
"否", //14
"请输入族谱名称('l'命令显示系统存在的族谱)", //15
"已存在，请重试", //16
"不存在，请重试", //17
"请输入父亲姓名", //18
"不在族谱中，请检查输入的名字是否有误。如需显示整个族谱，请按'p'或'd'命令。", //19
"不在族谱中，请检查输入的ID是否有误。如需显示整个族谱，请按'p'或'd'命令。", //20
"请输入孩子的名字", //21
"在族谱中已经存在，请再次确认输入的名字是否正确(y:是 其他:否)", //22
"请输入要添加配偶信息的宗亲的姓名", //23
"有重名，请输入ID(若退出此命令，输入q)", //24
"请输入配偶姓名", //25
"请重新添加", //26
"请输入性别(女输入0，男输入1)", //27
"请输入排行", //28
"请输入要修改的姓名", //29
"请输入新名", //30
"请输入姓名", //31
"请输入要删除的族谱名称", //32
"请输入族谱名称", //33
"父亲", //34
"族谱列表", //35
"在族谱中已存在此名字的配偶，请再次确认输入的名字是否正确(y:是 其他:否)", //36
"请重试。", //37
"请输入ID", //38
"删除失败", //39
"删除成功", //40
"请输入要删除的姓名", //41
"族谱已经存在。如需创建新的族谱，请保存当前族谱，退出再重新加载。", //42
"加载族谱成功。如需显示整个族谱，请按'p'或'd'命令。", //43
"的姓名已被更新为", //44
"的排行已被更新为", //45
"的性别已被更新为", //46
"此命令不支持", //47
"族谱还没有建立('l'命令显示系统存在的族谱,'r'命令进行加载; 或者'c'命令创建全新的族谱)", //48
"族谱列表为空", //49
"重名数超过了最大数", //50
"族谱中没有搜索到此姓名", //51
"搜索到的宗亲ID为", //52
"搜索到的宗亲配偶ID为", //53
"请输入备注信息(最多五百个字)", //54
"备注", //55
"请输入生成html文件的名字", //56
"如发现有错误之处，或者需要新增信息，请把准确信息发送至",//57
"世代", //58
"父系世代链如下", //59
"警告：不合理路径。行号", //60
"失败：参数错误。行号", //61
"失败：指针为空。行号", //62
"函数执行失败。行号", //63
"曾用名", //64
"族谱已经加载处于编辑状态，不能删除", //65
"欢迎使用,命令帮助,按'h'。", //66
"请输入初祖在上接父族谱中的世代数(如没有，请填0)", //67
"相对父族谱世代数", //68
"上接父族谱", //69
"下接子族谱", //70
"请输入上接父族谱的名字(输入NULL表示删除)", //71
"请输入下接子族谱的名字(输入NULL表示删除)", //72
"家谱具有极高的私密性，涉及整个家族隐私，请不要保留，不要扩散，不得外传。", //73
"输入错误，请重试", //74
"输入错误，请输入数字", //75
"当前已有族谱正在编辑状态，是否继续(y:是 其他:否)",//76
"是否保存当前正在编辑的族谱(n:否 其他:是)", //77
"已保存并关闭族谱", //78
"未保存并关闭族谱", //79
"族谱名称", //80
"族谱列表中不存在族谱", //81
"族谱已经加载处于编辑状态，继续则会自动保存当前修改并导出。是否继续(y:是 其他:否)", //82
"请输入导出的文件路径及文件名", //83
"系统中已存在同名族谱。继续加载需要一个新的族谱名字，是否继续(y:是 其他:否)", //84
"请输入导入的文件路径及文件名",//85
"-1:内容为空；-2:数据错误，内容长度太短；-3:数据错误，可能是解码字符串输入错误；-4:输入的ft文件已被破坏。", //86
"请设置最少6位最多16位字符作为编码字符串", //87
"解码字符串不正确，请重试", //88
"请输入解码字符串", //89
"编码字符串设置格式不正确,请重试", //90
"请输入新族谱名称", //91
"成功导出至文件", //92
"请再输入一遍", //93
"编码字符串两次输入不一致,请重试", //94
"ID对应的姓名与期望不符合,请重试", //95
"输入中不能包含特殊字符('#' ',' ';' '--'等),请重试", //96
"无效字符，请重试", //97
"族谱处于编辑状态，是否继续删除(y:是 其他:否)",//98
"男子数",//99
"女子数",//100
"输入中不能包含空格(' ' '\\t'等),请重试", //101
"执行失败。错误码",//102
"始祖",//103
"不能移动，已在最上(上移)/最下(下移)",//104
"存在不止一个初祖数据，族谱创建失败",//105
"导入族谱成功。如需显示整个族谱，请按'p'或'd'命令。", //106
};

char *msg_en[]={
"begin to create a new family tree. Please input a name(no space)",  //0
"please input the name of the first ancestor", //1
"male", //2
"female", //3
"already exist, please re-input", //4
"generation", //5
"sex", //6
"order", //7
"number of children", //8
"param is null", //9
"childre are", //10
"spouse", //11
"if repetition of name", //12
"yes", //13
"no", //14
"please input family tree name(command 'l' to show all family trees in system)", //15
"already exist, please retry", //16
"non-exist, please retry", //17
"please input the name of father", //18
"is not in family tree,please check inputted name. command 'p' or 'd' to show all informaiton in famiry tree.", //19
"is not in family tree,please check inputted ID.command 'p' or 'd' to show all informaiton in famiry tree.", //20
"please input name of child", //21
"already exists in family tree, please double check if the inputted name is righ(y for right, others for wrong)", //22
"please input the name", //23
"name repetition, please input ID('sbn' for searching ID for name)", //24
"please input the name of spouse", //25
"please re-add", //26
"please input sex(0 for female,1 for male)", //27
"please input order", //28
"please input the name to modify", //29
"please input new name", //30
"please input name", //31
"please input the name of family tree to be deleted", //32
"please input the name of family tree to be renamed", //33
"father", //34
"family tree list", //35
"alread exists in family tree, please double check if the inputted name is righ(y for right, others for wrong)", //36
"please retry.", //37
"please input ID", //38
"delete unsuccessfully", //39
"delete successfullye", //40
"please input the nane to be deleted", //41
"family tree already exists. if need to create new family tree, please save curernt one and reopen the program.", //42
"read in successfully", //43
"name has been updated to be", //44
"order has been updated to be", //45
"sex has been updated to be", //46
"unsupported", //47
"family tree is empty('l' to display family tree list in system, 'r' to load one; 'c' to create a new family tree))", //48
"family tree list is empty", //49
"the num of repetition of name is bigger than limitation", //50
"not found", //51
"num of people by search", //52
"ID are", //53
"please input note(max 1k characters)", //54
"note", //55
"please input name of html", //56
"if find any incorrect info or add new info, please email to",//57
"generation", //58
"paternal line link is", //59
"warning:invalid path. Line number is", //60
"fail:parameter wrong. Line number is", //61
"fail:pointer is null. Line number is", //62
"fail to run function. Line number is", //63
"used name", //64
"family tree is being status of edit, forbidden to be deleted.", //65
"Welcome, click 'h' for help.", //66
"please intput relative generation(0 for none)", //67
"relative generation", //68
"father family tree", //69
"child faimily tree", //70
"please input father family tree name(input NULL as deletion)", //71
"please input child family tree name(input NULL as deletion)", //72
"Highly private. Please don't keep it, don't spread it.", //73
"input erro, please retry", //74
"input error, please input number", //75
"family tree is editing status, continue?(y for yes, other for no)",//76
"save editing family tree?(n for no, other for yes)", //77
"closed with saving", //78
"closed without saving", //79
"family tree name", //80
"no such family tree in system family tree list", //81
"family tree is editing status, continue to save and export it?(y for yes, others for no)", //82
"please input export file name", //83
"existed name. need to name a new one, continue?(y for yes, others for no)", //84
"please input the imported file name",//85
"-1:content is empty; -2:lenth of content is wrong; -3:data error,please check decode string; -4:imported ft file was damaged", //86
"please input encode string(at lease 6 at most 16 characters)", //87
"decode string wrong,please retry", //88
"please input decode string", //89
"encode string format wrong,please retry", //90
"please input new family tree name", //91
"success to export", //92
"please input again", //93
"encode string input incompatible, please retry", //94
"related  name is not expected, please retry", //95
"no allow of control charaters('#' ',' ';' '--'),please retrry", //96
"invalid character, please retry", //97
"family tree is editing status, continue?(y for yes, others for no)",//98
"male number",//99
"female number",//100
"no allow of spaces(' ' '\\t'),please retry", //101
"failed. error code",//102
"ancestor",//103
"cannot move due to be already on top(for up) or bottom(for down)",//104
"not only one first ancestor data, failed to create family tree",//105
"import successfully",//106
};

char *help_zh[][2]={
               {"'help' 或者 'h'","帮助信息"}, //0
               {"'createfamilytree' 或者 'c'","创建新的族谱"}, //1
               {"'listfamilytrees' 或者 'l'","列出系统存在的族谱"}, //2
               {"'readfamilytree' 或者 'r'","加载一个系统存在的族谱"}, //3
               {"'savefamilytree' 或者 's'","保存编辑中的族谱数据到系统中"}, //4
               {"'renamefamilytree' 或者 'rf'","重命名一个系统中的族谱"}, //5
               {"'deletefamilytree' 或者 'df'","删除一个系统中族谱"}, //6
               {"'exportfamilytreetofile' 或者 'eft'","导出族谱到文件"}, //7
               {"'importfamilytreefromfile' 或者 'ift'","从文件导入族谱"}, //8
               {"'addchild' 或者 'ac'","添加子女"}, //9
               {"'addspouse' 或者 'as'","添加配偶"}, //10
               {"'updatename' 或者 'un'","修改姓名"}, //11
               {"'updatesex' 或者 'us'","修改性别"}, //12
               {"'updateorder' 或者 'uo'","修改排行"}, //13
               {"'updatenote' 或者 'unt'","修改备注"}, //14
               {"'printfamily' 或者 'pf'","显示整个族谱"}, //15
               {"'printfamilysimplest' 或者 'pfs'","显示整个族谱"}, //16
               {"'printfamilysimple' 或者 'p'","显示整个族谱"}, //17
               {"'printfamilydetails' 或者 'd'","显示整个族谱"}, //18
               {"'printchain' 或者 'pc'","显示某个人的父系链信息"}, //19
               {"'printfamilyhtml' 或者 'pfh'","将族谱按树形显示生成一个html文件"}, //20
               {"'printperson' 或者 'pp'","显示一个人的详细信息"}, //21
               {"'printpersonbyID' 或者 'pi'","显示一个人的详细信息"}, //22
               {"'printspouse' 或者 'ps'","显示配偶的详细信息"}, //23
               {"'printspousebyID' 或者 'psi'","显示配偶的详细信息"}, //24
               {"'searchbyname' 或者 'sbn'","根据名字搜索出所有此名字的ID"}, //25
               {"'deletesubtree' 或者 'dst'","删除一个分支"}, //26
               {"'updatenoteoffamilytree' 或者 'unf'","更新族谱备注信息"}, //27
               {"'printsubtreehtml' 或者 'psh'","将子族谱按树形显示生成一个html文件"}, //28
               {"'printsubtreesimple' 或者 'pss'","显示子族谱信息"}, //29
               {"'statistics' 或者 'st'","显示当前族谱统计信息"}, //30
               {"'statisticsofsubtree' 或者 'ss'","显示子族谱统计信息"}, //31
               {"'updateupft' 或者 'uu'","更新上接父族谱名字"}, //32
               {"'updatedownft' 或者 'ud'","更新下接子族谱名字"}, //33
               {"'updaterelativegeneration' 或者 'ur'","更新相对世代数"}, //34
               {"'moveup' 或者 'mu'","排列向上移动"},//35
               {"'movedown' 或者 'md'","排列向下移动"},//36
               {"'quit' 或者 'q'","退出"}, //37
               {"\0","\0"}, //结束标记，确保在最后一行
};

char *help_en[][2]={
               {"'help' or 'h'","help information"}, //0
               {"'createfamilytree' or 'c'","create a new family tree"}, //1
               {"'readfamilytree' or 'r'","read family tree from system"}, //2
               {"'savefamilytree' or 's'","write family tree to system"}, //3
               {"'listfamilytrees' or 'l'","list family trees in system"}, //4
               {"'renamefamilytree' or 'rf'","rename a family tree in system"}, //5
               {"'deletefamilytree' or 'df'","delete a family tree from system"}, //6
               {"'exportfamilytreetofile' or 'eft'","export family tree to a ft file"}, //7
               {"'importfamilytreefromfile' or 'ift'","import family tree from a ft file"}, //8
               {"'addchild' or 'ac'","add a child"}, //9
               {"'addspouse' or 'as'","add a spouse"}, //10
               {"'updatename' or 'un'","update name of a person"}, //11
               {"'updatesex' or 'us'","update sex of a person"}, //12
               {"'updateorder' or 'uo'","update order of a person"}, //13
               {"'updatenote' or 'unt'","update note of a person"}, //14
               {"'printfamily' or 'pf'","display all members of a family tree"}, //15
               {"'printfamilysimplest' or 'p'","display all members of a family tree"}, //16
               {"'printfamilysimple' or 'pfs'","display all members of a family tree"}, //17
               {"'printfamilydetails' or 'd'","display all members of a family tree"}, //18
               {"'printchain' or 'pc'","display the paternal chain of a person"}, //19
               {"'printfamilyhtml' or 'pfh'","print family tree to a html file with tree-style"}, //20
               {"'printperson' or 'pp'","display information of a person"}, //21
               {"'printpersonbyID' or 'pi'","display information of a person"}, //22
               {"'printspouse' or 'ps'","display information of spouse of a person"}, //23
               {"'printspousebyID' or 'psi'","display information of spouse of a person"}, //24
               {"'searchbyname' or 'sbn'","search all IDs for a given name"}, //25
               {"'deletesubtree' or 'dst'","delete sub tree of a family tree"}, //26
               {"'updatenoteoffamilytree' or 'unf'","update note for a family tree "}, //27
               {"'printsubtreehtml' or 'psh'","print a subtree to html file with tree-style"}, //28
               {"'printsubtreesimple' or 'pss'","display subtree"}, //29
               {"'statistics' or 'stat'","display statistics of a family tree"}, //30
               {"'statisticsofsubtree' or 'ss'","dispplay stattistics of a sub family tree"}, //31
               {"'updateupft' or 'uu'","update father family tree of a person"}, //32
               {"'updatedownft' or 'ud'","update child family tree of a person"}, //33
               {"'updaterelativegeneration' or 'ur'","update relative generation"}, //34
               {"'moveup' or 'mu'","move up"},//35
               {"'movedown' or 'md'","move down"},//36
               {"'quit' or 'q'","quit"}, //37
               {"\0","\0"}, //terminator, ensure to be last line
};

char **msg=msg_zh;
char *(*help)[2]=help_zh;
char *headerword="#EFTFF#";

uint crcTable[]= {
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c66d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

int getNowDateStr(char *retStr,int size,int type){
   if(NULL==retStr||size<DATE_LEN) {printf("%s:%d",msg[61],__LINE__);return -1;}
   time_t now;
   time(&now);
   struct tm * timeinfo;
   timeinfo = localtime (&now);
   if(1==type)
       sprintf(retStr,"%04d-%02d-%02d %02d:%02d:%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
   if(2==type)
       sprintf(retStr,"%04d-%02d-%02d_%02d:%02d:%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
   if(3==type)
       sprintf(retStr,"%04d-%02d-%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday);
   if(4==type)
       sprintf(retStr,"%02d:%02d:%02d",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

   return 0;
}

#define printdebug(format,value) do { \
     char now_date_debug[DATE_LEN]={0}; \
     getNowDateStr(now_date_debug,DATE_LEN,1); \
     printf("\n%s function:%s line:%d ",now_date_debug,__func__,__LINE__); \
     printf(format,value); \
} while(0)

uint crc_simple(char *str, int nBytes)
{
    uint crc = 0xffffffffu;
    for (int i = 0; i < nBytes; i++) crc=crcTable[(crc ^ str[i]) & 0xff] ^ (crc >> 8);
    crc^=0xffffffffu;
    return crc;
}

void initSystemData()
{
    g_tree_root=NULL;
    g_id=1;
    memset(ftn,0,sizeof(ftn));
    memset(ft_note,0,sizeof(ft_note));
    return;
}

Person_t * getPersonByID(ulong id,Person_t * searchTree)
{
   Person_t * retP;
   if(NULL==searchTree){return NULL;}
   if(id==searchTree->id){
       return searchTree;
   }else if(searchTree->n_childreninclan){
       for(int i=0;i<searchTree->n_childreninclan;i++)
       {
          retP=getPersonByID(id,searchTree->childreninclan[i]);
          if(retP) return retP;
       }
       return NULL;
   }else{
       return NULL;
   }
}

PersonInLaw_t * getSpouseByID(ulong id,Person_t * searchTree)
{
   PersonInLaw_t * retP;
   if(NULL==searchTree){return NULL;}
   int i;
   for(i=0;i<searchTree->n_spouse;i++){
     if(id==searchTree->spouse[i]->id){
        return searchTree->spouse[i];
     }
   }

   if(searchTree->n_childreninclan){
       for(i=0;i<searchTree->n_childreninclan;i++)
       {
          retP=getSpouseByID(id,searchTree->childreninclan[i]);
          if(retP) return retP;
       }
       return NULL;
   }else{
       return NULL;
   }
}

ulong getID(){
   while(getPersonByID(g_id,g_tree_root)||getSpouseByID(g_id,g_tree_root)){
      g_id++;
   }
   return g_id;
}

int isAllDigits(char * str){
    for(int i=0;i<strlen(str);i++) if(!isdigit(str[i])) return 0;
    return 1;
}

int isIncludeControlChars(char * str){
   if(NULL==str) return 0;
   char *ctl_chars[]={
      "#",
      ",",
      ";",
      "#@",
      "@#",
      "--",
      "#EFTFF#",
   };
   for(int i=0;i<sizeof(ctl_chars)/sizeof(ctl_chars[0]);i++){
      if(strstr(str,ctl_chars[i])) return 1;
   }
   return 0;
}

int getchar_echo(int echo){
   struct termios old, new;
   tcgetattr( STDIN_FILENO, &old );
   new = old;
   new.c_lflag &= ~ICANON;
   if(echo) new.c_lflag &=  ECHO;
   else new.c_lflag &= ~ECHO;
   tcsetattr( STDIN_FILENO, TCSANOW, &new );
   int ch = getchar();
   tcsetattr( STDIN_FILENO, TCSANOW, &old );
   return ch;
}

int getch(){
   return getchar_echo(0);
}
      
int getche(){
   return getchar_echo(1);
}

void scanf_string_noecho(char * fmt,char * strstore){
   struct termios old, new;
   tcgetattr( STDIN_FILENO, &old );
   new = old;
   new.c_lflag &= ~ECHO;
   tcsetattr( STDIN_FILENO, TCSANOW, &new );
   scanf(fmt,strstore);
   tcsetattr( STDIN_FILENO, TCSANOW, &old );
   return ;
}

int getPass(char *pass,int maxsize){
   char fmt[16]={0};
   sprintf(fmt," %s%d[^\n]","%",maxsize);
   fflush(stdin);
   scanf_string_noecho(fmt,pass);
   return 0;
}

/*store "maxsize" charaters at most into "str". Ensure "str" remains one more byte for \0. scan input until \n not space. */
void scanfStr(char *str,int maxsize){
   char format[128]={0};
   sprintf(format," %s%d[^\n]","%",maxsize);
   while('\0'==str[0]){ 
      fflush(stdin);
      scanf(format,str);
   }
   return;
}

int isSpaceInString(char * str,int size){
   if(NULL==str) return 0;
   for(int i=0;i<size;i++) if(isspace(str[i])) return 1;
   return 0;
}

void trimStr(char *str){
   if((NULL==str)||(strlen(str)<1)) return;
   int flag=0,i=0;
   for(i=strlen(str)-1;i>=0;i--) if(!isspace(str[i])) {flag=1;break;}
   if(flag) str[++i]='\0'; else {str[0]='\0'; return;}

   char *p=str;
   char *ph=str;
   while((isspace(*p))&&(*p!='\0')) p++;
   while(*p!='\0') *ph++=*p++;
   *ph='\0';
   return;
}

void serializeFamilyTreeToFile(Person_t * top,FILE *fp)
{
   if(top==NULL){printf("%s:%d\n",msg[61],__LINE__);return;}
   if(fp) fprintf(fp,"%s#%ld#%ld#%s#%s#%hd#%hd#%hd#%hd#%ld",top->name,top->generation,top->relative_generation,top->up_ft,top->down_ft,top->order,top->sex,top->n_childreninclan,top->n_spouse,top->id);
   else printf("%s#%ld#%ld#%s#%s#%hd#%hd#%hd#%hd#%ld",top->name,top->generation,top->relative_generation,top->up_ft,top->down_ft,top->order,top->sex,top->n_childreninclan,top->n_spouse,top->id);

   for(int i=0;i<top->n_spouse;i++){
      if(fp) fprintf(fp,",%s#%ld#%hd",top->spouse[i]->name,top->spouse[i]->id,top->spouse[i]->sex);
      else printf(",%s#%ld#%hd",top->spouse[i]->name,top->spouse[i]->id,top->spouse[i]->sex);
   }
   if(strlen(top->note)>0){
      if(fp) fprintf(fp,";%s#",top->note); else printf(";%s#",top->note);
   }
   if(fp) fprintf(fp,"\n"); else printf("\n");
   for(int i=0;i<top->n_childreninclan;i++){
      if(fp) fprintf(fp,"%ld--",top->id); else printf("%ld--",top->id);
      serializeFamilyTreeToFile(top->childreninclan[i],fp);
   }
   return;
}

int writeFamilyTreeToSystemFile()
{
   if(g_tree_root==NULL){printf("%s\n",msg[48]);return -1;}
   time_t now;
   time(&now);
   char storepath[PATH_LEN]={0};
   char versionspath[PATH_LEN]={0};
   char defaultpath[PATH_LEN]={0};
   char storefile[PATH_LEN]={0};
   char path[PATH_LEN]={0};

   if(access(".familytrees",0)){
      mkdir(".familytrees",0777);
   }

   sprintf(path,".familytrees/%s",ftn);
   if(access(path,0)){
      mkdir(path,0777);
   }

   sprintf(storefile,"data%ld.ftd",now);
   sprintf(storepath,"%s/%s",path,storefile);
   FILE *fp = NULL;
   fp = fopen(storepath, "w");
   if(fp==NULL){printf("%s:%d\n",msg[62],__LINE__);return -100;}
   serializeFamilyTreeToFile(g_tree_root,fp);
   fclose(fp);

   char now_date[DATE_LEN]={0};
   getNowDateStr(now_date,DATE_LEN,1);
   sprintf(versionspath,".familytrees/%s/versions.ftv",ftn);
   fp = fopen(versionspath,"a");
   fprintf(fp,"%s %s\n",storefile,now_date);
   fclose(fp);
   sprintf(defaultpath,".familytrees/%s/data.ftd",ftn);
   unlink(defaultpath);
   symlink(storefile,defaultpath);

   return 0;
}

int createFamilyTree()
{
   if(g_tree_root){
      printf("\n %s",msg[42]);
      return -1;
   }
   char path[PATH_LEN]={0};
   char name[NAME_LEN]={0};

   if(access(".familytrees",0)){
      mkdir(".familytrees",0777);
   }

   printf("%s:",msg[0]);
   memset(ftn,0,sizeof(ftn));
   scanfStr(ftn,sizeof(ftn)-1);
   trimStr(ftn);
   if('\0'==ftn[0]) {printf("%s\n",msg[97]);return -1;}
   if(isSpaceInString(ftn,strlen(ftn))) {memset(ftn,0,sizeof(ftn));printf("%s\n",msg[101]);return -1;} 
   if(isIncludeControlChars(ftn)) {memset(ftn,0,sizeof(ftn));printf("%s\n",msg[96]);return -1;}

   sprintf(path,".familytrees/%s",ftn);
   if(mkdir(path,0777)) {printf("%s %s\n",ftn,msg[16]);return -1;}

   printf("%s:",msg[1]);
   scanfStr(name,(sizeof(name)-1));
   trimStr(name);
   if(isIncludeControlChars(name)) {printf("%s\n",msg[96]);return -1;}

   g_tree_root=(Person_t *)malloc(sizeof(Person_t));
   memset(g_tree_root,0,sizeof(Person_t));
   strcpy(g_tree_root->name,name);

   printf("%s:",msg[67]);
   char intstr[BUFF_LEN]={0};
   scanf("%s",intstr);
   if(isAllDigits(intstr)){
      g_tree_root->relative_generation=atoi(intstr);
   }else{
      free(g_tree_root);
      initSystemData();
      remove(path);
      printf("%s\n",msg[75]);
      return -1;
   }

   char now_date[DATE_LEN]={0};
   getNowDateStr(now_date,DATE_LEN,1);
   FILE *fp = fopen(".familytrees/.familytreeslist.ftl","a");
   if(fp==NULL){printf("%s:%d\n",msg[62],__LINE__);return -2;}
   fprintf(fp,"%s %s \n",ftn,now_date);
   fclose(fp);

   strcpy(g_tree_root->up_ft,"NULL");
   strcpy(g_tree_root->down_ft,"NULL");
   g_tree_root->sex = 1; 
   g_tree_root->id=getID();
   g_tree_root->generation=1;
   g_tree_root->order=1;

   writeFamilyTreeToSystemFile();

   return 0;
}

int deleteSubTreeInternel(Person_t * node)
{
    if(node==NULL){printf("%s:%d",msg[61],__LINE__);;return -1;}
    int i;
    for(i=0;i<node->n_spouse;i++){
       if(node->spouse[i]){free(node->spouse[i]);node->spouse[i]=NULL;}
    }
    if(node->spouse){free(node->spouse);node->spouse=NULL;}
    node->n_spouse=0;
    for(i=0;i<node->n_childreninclan;i++){
       if(node->childreninclan[i]) deleteSubTreeInternel(node->childreninclan[i]);
    }
    if(node->father){
       if(node->father->n_childreninclan>0){
          for(i=0;i<node->father->n_childreninclan;i++){
             if((node->father->childreninclan[i])&&(node->father->childreninclan[i]->id==node->id)){
                node->father->childreninclan[i]=NULL;
              }
           }
       }else{
           printf("%s:%d",msg[60],__LINE__);
       }
    }else{
       g_tree_root=NULL;
    }
    
    if(node->childreninclan){free(node->childreninclan);node->childreninclan=NULL;}
    if(node){free(node);node=NULL;}
    return 0;
}

int deleteSubTree(Person_t * node)
{
    if(node==NULL){printf("%s:%d",msg[61],__LINE__);return -1;}
    Person_t *father = node->father;
    ulong id=node->id;
    int i,newi;
    deleteSubTreeInternel(node);
    if(father==NULL)return 0;
    
    if(father->n_childreninclan){
       if(father->n_childreninclan>1){
          Person_t **p_per=(Person_t **)malloc((father->n_childreninclan-1)*sizeof(Person_t *));
          for(i=0,newi=0;i<father->n_childreninclan;i++){
             if((father->childreninclan[i])&&(father->childreninclan[i]->id!=id)){
                p_per[newi++]=father->childreninclan[i];
             }
          }
          if(father->childreninclan) free(father->childreninclan);
          father->childreninclan=p_per;
          father->n_childreninclan-=1;
       }else{
          if(father->childreninclan) {free(father->childreninclan);father->childreninclan=NULL; }
          father->n_childreninclan=0;
       }
    }else{
       printf("%s:%d",msg[60],__LINE__);
    }  
    return 0;
}

int updateNameOfPerson(Person_t * per,char * name){
    printf("%s(ID:%ld) %s %s",per->name,per->id,msg[44],name);
    strcpy(per->name,name);
    return 0;
}

int updateOrderOfPerson(Person_t * per,ushort order){
    per->order=order;
    printf("%s(ID:%ld) %s %d",per->name,per->id,msg[45],order);
    return 0;
}

int updateSexOfPerson(Person_t * per,ushort sex){
   per->sex=sex;
   printf("%s(ID:%ld) %s %hu",per->name,per->id,msg[46],sex);
   return 0;
}


int addChild(Person_t *father,Person_t *child)
{
    if(father==NULL || child==NULL){printf("\n%s:%d",msg[61],__LINE__);return -1;}
    Person_t **p_per=(Person_t **)malloc((father->n_childreninclan+1)*sizeof(Person_t *));
    int i;
    for(i=0;i<father->n_childreninclan;i++){
       p_per[i]=father->childreninclan[i];
    }
    child->father=father;
    child->generation=father->generation+1;
    if(father->relative_generation)child->relative_generation=father->relative_generation+1;
    
    p_per[i]=child;

    if(father->childreninclan) free(father->childreninclan);
    father->childreninclan=p_per;
    father->n_childreninclan+=1;

    return 0;
}

int addSpouse(Person_t *spoused,PersonInLaw_t *spouse)
{
    if(spoused==NULL || spouse==NULL){printf("\n%s:%d",msg[61],__LINE__);return -1;}
    PersonInLaw_t ** p_perIL=(PersonInLaw_t **)malloc((spoused->n_spouse+1)*sizeof(PersonInLaw_t *));
    int i;
    for(i=0;i<spoused->n_spouse;i++){
       p_perIL[i]=spoused->spouse[i];
    }
    spouse->relation=spoused;
    p_perIL[i]=spouse;
    if(spoused->spouse)free(spoused->spouse);
    spoused->spouse=p_perIL;
    spoused->n_spouse+=1;

    return 0;
}

PersonInLaw_t * getSpouseByName(char * name,Person_t * searchTree)
{
   PersonInLaw_t * retP;
   if(searchTree==NULL){return NULL;}
   int i;
   if(searchTree->n_spouse){
      for(i=0;i<searchTree->n_spouse;i++){
         if(strcmp(name,searchTree->spouse[i]->name)==0){
            return searchTree->spouse[i];
         }
      }
   } 

   if(searchTree->n_childreninclan){
       for(i=0;i<searchTree->n_childreninclan;i++)
       {
          retP=getSpouseByName(name,searchTree->childreninclan[i]);
          if(retP) return retP;
       }
       return NULL;
   }else{
       return NULL;
   }
}

Person_t * getPersonByName(char * name,Person_t * searchTree)
{
   Person_t * retP;
   if(NULL==searchTree) return NULL;
   if(0==strcmp(name,searchTree->name)) return searchTree;
   else if(searchTree->n_childreninclan!=0){
       for(int i=0;i<searchTree->n_childreninclan;i++)
       {
          retP=getPersonByName(name,searchTree->childreninclan[i]);
          if(retP) return retP;
       }
       return NULL; 
   }else{
       return NULL;
   }
}

void printPerson(Person_t * per){
   if(per==NULL){
      printf("%s.",msg[9]);
      return;
   }
   printf("*******************************   ");
   if(per->sex==1)printf("%s(%s)",per->name,msg[2]); 
   else if(per->sex==0)printf("%s(%s)",per->name,msg[3]);
   else printf("%s",per->name);
   printf("   ********************************"); 
   printf("\n%s:%lu",msg[5],per->generation);
   printf("\n%s:%lu",msg[68],per->relative_generation);
   if(strcmp(per->up_ft,"NULL"))printf("\n%s:%s",msg[69],per->up_ft);   
   if(strcmp(per->down_ft,"NULL"))printf("\n%s:%s",msg[70],per->down_ft);
   printf("\n%s:%hu",msg[7],per->order);
   printf("\nID:%lu",per->id);
   if(per->father)printf("\n%s:%s(ID:%ld)",msg[34],per->father->name,per->father->id);
   printf("\n%s:%hu",msg[8],per->n_childreninclan);
   if(per->n_childreninclan!=0){
      printf("\n%s:",msg[10]);
      for(int i=0;i<per->n_childreninclan;i++){
         if(per->childreninclan[i]->sex==1)printf("%s(%s) ",per->childreninclan[i]->name,msg[2]);
         else if(per->childreninclan[i]->sex==0)printf("%s(%s) ",per->childreninclan[i]->name,msg[3]);
         else printf("%s ",per->childreninclan[i]->name);
      }
   }
   if(per->n_spouse!=0){
     printf("\n%s:",msg[11]);
     for(int i=0;i<per->n_spouse;i++){
        printf("%s ",per->spouse[i]->name);
     }
   }
   if(strlen(per->note))printf("\n%s:%s",msg[55],per->note);
   printf("\n");
}

void printPersonInLaw(PersonInLaw_t *perIL)
{
   if(perIL==NULL){
      printf("%s.",msg[9]);
      return;
   }
   printf("*******************************   ");
   if(perIL->sex==1)printf("%s(%s)",perIL->name,msg[2]); 
   else if(perIL->sex==0)printf("%s(%s)",perIL->name,msg[3]);
   else printf("%s",perIL->name);
   printf("   ********************************");
   printf("\nID:%lu",perIL->id);
   printf("\n%s:%s(ID:%lu)",msg[11],perIL->relation->name,perIL->relation->id);
   printf("\n");

   return;
}

void printFamilyTreeStyleToHtmlFileInternel(FILE *fp,Person_t * tree,ushort init_gen){
   if(fp==NULL){printf("%s:%d\n",msg[61],__LINE__);return;}
   if(tree==NULL){fprintf(fp,"%s\n",msg[48]);return;}
   if(init_gen<1){printf("%s\n",msg[61]);return;}
   int i,j;
   Person_t *p;

   tree->n_childreninclan_print=tree->n_childreninclan;
   for(i=1;i<tree->generation-(init_gen-1);i++){
      p = tree;
      for(j=tree->generation-(init_gen-1);j>i;j--){
          p=p->father;
      }
      if(p->n_childreninclan_print){
         fprintf(fp,"&nbsp;&#160;&nbsp;&#160;&nbsp;&#160;&nbsp;&#160;|");
      }else{
         fprintf(fp,"&nbsp;&#160;&nbsp;&#160;&nbsp;&#160;&nbsp;&#160;&nbsp;&#160;");
      }
   }
   if(tree->father) tree->father->n_childreninclan_print--;

   fprintf(fp,"____%s",tree->name);
   if(strlen(tree->note)>0)fprintf(fp,"(%s)",tree->note);
   for(i=0;i<tree->n_spouse;i++){
      fprintf(fp," %s",tree->spouse[i]->name);
   }
   fprintf(fp,"<br />");
   for(i=0;i<tree->n_childreninclan;i++){
      printFamilyTreeStyleToHtmlFileInternel(fp,tree->childreninclan[i],init_gen);
   }
}

void printFamilyTreeStyleToHtmlFile(){
   if(g_tree_root==NULL){printf("%s\n",msg[48]);return;}
   char name[NAME_LEN]={0};
   char path[PATH_LEN]={0};
   printf("%s:",msg[56]);
   scanf("%s",name);
   sprintf(path,"./%s.html",name);
   FILE *fp=fopen(path,"w");
   if(fp==NULL){printf("%s:%d\n",msg[62],__LINE__);return;}
   fprintf(fp,"<html>");
   fprintf(fp,"<p><br /><b>%s</b></p><br />",msg[73]);
   fprintf(fp,"<h1 align=\"center\">%s</h1><br />",ftn);
   fprintf(fp,"<body style=\"overflow:auto;white-space:nowrap;\">");
   fprintf(fp,"<p>");

   printFamilyTreeStyleToHtmlFileInternel(fp,g_tree_root,g_tree_root->generation);

   fprintf(fp,"</p>");
   fprintf(fp,"<br /><p>%s</p><br /><br />",ft_note);
   fprintf(fp,"</body>");
   fprintf(fp,"</html>");

   fclose(fp);
}

void printSubtreeToHtmlFile(char *html_name,Person_t *per){
   char path[PATH_LEN]={0};
   sprintf(path,"./%s.html",html_name);
   FILE *fp=fopen(path,"w");
   if(fp==NULL){printf("%s:%d\n",msg[62],__LINE__);return;}
   fprintf(fp,"<html>");
   fprintf(fp,"<body style=\"overflow:auto;white-space:nowrap;\">");
   fprintf(fp,"<p>");

   printFamilyTreeStyleToHtmlFileInternel(fp,per,per->generation);

   fprintf(fp,"</p>");
   fprintf(fp,"<br /><p>%s</p><br /><br />",ft_note);
   fprintf(fp,"</body>");
   fprintf(fp,"</html>");

   fclose(fp);
}

void printFamilyTreeInternal(Person_t * tree,int init_gen,int type){
   int i,j;
   Person_t *p;

   tree->n_childreninclan_print=tree->n_childreninclan;
   for(i=1;i<tree->generation-(init_gen-1);i++){
      p = tree;
      for(j=tree->generation-(init_gen-1);j>i;j--){
          p=p->father;
      }
      if(p->n_childreninclan_print){
         printf("   |");
      }else{
         printf("    ");
      }
   }
   if(tree->father) tree->father->n_childreninclan_print--;
   
   printf("___%s",tree->name);
   if(PRINTALL==type) {
      printf(" (");
      if(tree->sex==0){
         printf("%s,",msg[3]);
      }
   }
   if(PRINTNAMESEX==type){
      if(tree->sex==0){
         printf("(%s)",msg[3]);
      }
   }
   if(PRINTALL==type){
      printf("%s:%lu,",msg[5],tree->generation);
      if(tree->relative_generation)printf("%s:%lu,",msg[68],tree->relative_generation);
      printf("%s:%d,%s:%hu,ID:%ld",msg[7],tree->order,msg[8],tree->n_childreninclan,tree->id);
      if(strcmp(tree->up_ft,"NULL"))printf(",%s:%s",msg[69],tree->up_ft);
      if(strcmp(tree->down_ft,"NULL"))printf(",%s:%s",msg[70],tree->down_ft);
      if(tree->n_spouse>0)printf(",%s:",msg[11]);
      for(i=0;i<tree->n_spouse;i++){
         if(tree->spouse[i]->sex==0){
            printf("%s<%s,ID:%ld>",tree->spouse[i]->name,msg[3],tree->spouse[i]->id);
         }else if(tree->spouse[i]->sex==1){
            printf("%s<%s,ID:%ld>",tree->spouse[i]->name,msg[2],tree->spouse[i]->id);
         }
      }
   }
   if(PRINTALL==type){
      if(strlen(tree->note)>0)printf(",%s:%s",msg[55],tree->note);
      printf(")");
   }
   if(PRINTNAMENOTE==type){
      if(strlen(tree->note)>0)printf(" (%s)",tree->note);
      for(i=0;i<tree->n_spouse;i++){
         printf(" %s",tree->spouse[i]->name);
      }
   }
   printf("\n");
   for(i=0;i<tree->n_childreninclan;i++){
      printFamilyTreeInternal(tree->childreninclan[i],init_gen,type);
   }
}

void printFamilyTree(Person_t * tree,int type){
   if(tree==NULL){printf("%s\n",msg[48]);return;}
   if(tree->generation<1){printf("%s\n",msg[61]);return;}
   printFamilyTreeInternal(tree,tree->generation,type);
}

int isTheLine(char *buff_line,char *ft_name)
{
   if(buff_line==NULL || ft_name==NULL) return 0;
   char cmp_name[NAME_LEN]={0};
   char *p;
   sprintf(cmp_name,"%s ",ft_name);
   if((p=strstr(buff_line,cmp_name))&&(p==buff_line)) return 1;
   else return 0;
}

void confuseString(char *str, int size,char *keystr){
   int len=strlen(keystr);
   if(len<=0) return;
   for(int i=0,j=0;i<size;i++,j++){
       str[i]=str[i]^keystr[j%len];
   }
   return;
}

int exportFamilyTreeToFile(char * ft_name,char * exp_path,char * keystr)
{
   FILE *fpl = NULL;
   FILE *fpd = NULL;
   FILE *fpe = NULL;
   int flag=1;
   char buff[BUFF_LEN]={0};
   char buffwrite[BUFF_LEN]={0};
   char buffhead[BUFF_LEN]={0};
   char defaultpath[PATH_LEN]={0};
   char path[PATH_LEN]={0};
   int writesize=0;
   int headsize=0;
   int fullsize=0;
   char *fullstr;
   char *p;

   if(strlen(exp_path)<strlen(".ft")||strcmp(exp_path+(strlen(exp_path)-strlen(".ft")),".ft")){
      sprintf(path,"%s.ft",exp_path);
   }else{
      sprintf(path,"%s",exp_path);
   }

   fpl = fopen(".familytrees/.familytreeslist.ftl", "r");
   if(fpl==NULL){printf("%s \n",msg[49]);return -2;}
   while(fgets(buff, BUFF_LEN, (FILE*)fpl)!=NULL){
      if(isTheLine(buff,ft_name)){
          flag=0;
          sprintf(buffhead,"%s%s",headerword,buff);
          headsize=strlen(buffhead);
          break;
      }
   }
   fclose(fpl);

   if(flag){printf("%s: %s\n",msg[81],ft_name);return -3;}

   sprintf(defaultpath,".familytrees/%s/data.ftd",ft_name);
   fpd = fopen(defaultpath, "r");
   fullsize+=headsize;
   while(fgets(buff, BUFF_LEN, (FILE*)fpd)!=NULL){
        fullsize+=strlen(buff);
   }
   fullsize+=CRCLEN;  
   fullstr=(char *)malloc(fullsize+BUFF_PAD);   
   memset(fullstr,0,sizeof(fullsize+BUFF_PAD));
   char *fullcontent=fullstr+CRCLEN;
   strcpy(fullcontent,buffhead);
   fseek(fpd,0,SEEK_SET);
   while(fgets(buff, BUFF_LEN, (FILE*)fpd)!=NULL){
      strcat(fullcontent,buff);
   }
   fclose(fpd);

   uint crc=crc_simple(fullstr+CRCLEN,fullsize-CRCLEN);
   memcpy(fullstr,&crc,CRCLEN);
#ifdef FTDEBUG
   printdebug("size of content is 0x%x",fullsize-CRCLEN);
   printdebug("content is\n%s",fullstr+CRCLEN);
#endif
   confuseString(fullstr,fullsize,keystr);
   fpe=fopen(path,"w");
   if(fpe==NULL){printf("%s:%s%d \n",path,msg[61],__LINE__);free(fullstr);return -1;}
   fwrite(fullstr,fullsize,1,fpe);
   fclose(fpe);
   free(fullstr);
   printf("%s:%s\n",msg[92],path);
   return 0;
}

int getFTinfoFromRecord(char * record)
{
   if(NULL==record){printf("%s%d \n",msg[61],__LINE__);return -1;}
   char *p;
   char str[BUFF_LEN];
   char ft_info[BUFF_LEN];
   if(strstr(record,headerword)){
      strcpy(ft_info,record+strlen(headerword));
      if((p=strstr(ft_info,"@#"))){
         p=p+2;
         strcpy(ft_note,p);
      }
      if((p=strstr(ft_info," "))){
         *p='\0';
         strcpy(ftn,ft_info);
      }
   }else{
      printf("%s%d\n",msg[61],__LINE__);
      return -2;
   }
   return 0;
}

Person_t * getPersonFromRecord(char * record)
{
      if(NULL==record){printf("%s%d \n",msg[61],__LINE__);return NULL;}
#ifdef FTDEBUG
      printdebug("record is %s",record);
#endif
      ulong father_id;
      int n;
      int i,j;
      char *p;
      PersonInLaw_t * perIL;
      PersonInLaw_t * * p_perIL;
      Person_t * per=(Person_t *)malloc(sizeof(Person_t));
      memset(per,0,sizeof(Person_t));
      if(strstr(record,headerword)){
         //ignore this line of record. this record is for family tree description information.
         return NULL;
      }else if(strstr(record,"--")){
         n=sscanf(record,"%ld--%[^#]#%ld#%ld#%[^#]#%[^#]#%hd#%hd#%hd#%hd#%ld",&father_id,per->name,&per->generation,&per->relative_generation,per->up_ft,per->down_ft,&per->order,&per->sex,&per->n_childreninclan,&per->n_spouse,&per->id);
         if(n!=11) {printf("%s%d \n",msg[62],__LINE__);free(per);return NULL;}
         per->father=getPersonByID(father_id,g_tree_root);
#ifdef FTDEBUG
         printdebug("father_id is %lu\n",father_id);
         printdebug("father ptr is %p\n",per->father);
#endif
      }else{
         n=sscanf(record,"%[^#]#%ld#%ld#%[^#]#%[^#]#%hd#%hd#%hd#%hd#%ld",per->name,&per->generation,&per->relative_generation,per->up_ft,per->down_ft,&per->order,&per->sex,&per->n_childreninclan,&per->n_spouse,&per->id);
         if(n!=10) {printf("%s:%d \n",msg[62],__LINE__);free(per);return NULL;}
      }

      if(strchr(record,',')){
         if(0==per->n_spouse){printf("\n%s:%d",msg[60],__LINE__); return NULL;}
         p_perIL=(PersonInLaw_t **)malloc(per->n_spouse*sizeof(PersonInLaw_t *));
         p=record;
         for(i=0;i<per->n_spouse;i++){
               p=strchr(p, ',');
               p=p+1;
               perIL=(PersonInLaw_t *)malloc(sizeof(PersonInLaw_t));
               n=sscanf(p,"%[^#]#%ld#%hd",perIL->name,&perIL->id,&perIL->sex);
               if(n!=3) {
                  printf("%s%d \n",msg[62],__LINE__);
                  free(perIL);
                  for(j=0;j<per->n_spouse;j++) if(p_perIL[j]!=NULL) free(p_perIL[j]);
                  free(p_perIL);
                  free(per);
                  return NULL;
               }
               perIL->relation=per;
               p_perIL[i]=perIL;
          }
          per->spouse=p_perIL;
      }

      if((p=strchr(record,';'))){
         p=p+1;
         n=sscanf(p,"%[^#]",per->note);
         if(n!=1) printf("\n%s:%d",msg[60],__LINE__);
     }

      per->n_childreninclan=0; //the number should be caculated when to add his child.

      return per;
}

int checkdata(char *data,int len){
   if(NULL==data) return -1;
   if(len<(strlen(headerword)+CRCLEN)) return -2;
   char * con=data+CRCLEN;
   for(int i=0;i<strlen(headerword);i++) if(con[i]!=headerword[i]) return -3;
   uint crc1=*(uint *)data;
   uint crc2=crc_simple(data+CRCLEN,len-CRCLEN);
#ifdef FTDEBUG
   printdebug("crc1 is %u",crc1);
   printdebug("crc2 is %u\n",crc2);
#endif
   if(crc1!=crc2) return -4;
   return 0;
}

int importFamilyTreeFromFile(char *imp_path,char *keystr)
{
   FILE *fp=NULL;
   char path[PATH_LEN]={0};
   char str[BUFF_LEN]={0};
   char buff[BUFF_LEN]={0};
   char ftinfo[BUFF_LEN]={0};
   char ync[NAME_LEN]={0};
   char *fullstr=NULL;
   Person_t * per=NULL;
   char *p=NULL;
   char *p0=NULL;
   int namechangeflag=0;
   int rc=0;

   fp=fopen(imp_path,"r");
   if(fp==NULL){printf("%s:%s\n",imp_path,msg[17]);return -1;}
   int fullsize=0;
   int size=0;
   while((size=fread(buff,1,BUFF_LEN,fp))) fullsize+=size;
   if(fullsize) fullstr=(char *)malloc(fullsize+BUFF_PAD);
   memset(fullstr,0,sizeof(fullsize+BUFF_PAD));
   p=fullstr;
   fseek(fp,0,SEEK_SET);
   while((size=fread(buff,1,BUFF_LEN,fp))){
      memcpy(p,buff,size);
      p+=size;
   }
   confuseString(fullstr,fullsize,keystr);
#ifdef FTDEBUG
   printdebug("size of content is 0x%x",fullsize-CRCLEN);
   printdebug("content is \n%s",fullstr+CRCLEN);
#endif
   rc=checkdata(fullstr,fullsize);
   if(rc) {printf("%s:%d [%s]\n",msg[102],rc,msg[86]);fclose(fp);return -10;}
   p=fullstr+CRCLEN;
   fullsize-=CRCLEN;
   while((p0=strchr(p,'\n'))){
      *p0='\0';
      strcpy(buff,p);
      p=p0+1;

      if(strstr(buff,headerword)){
          strcpy(ftinfo,buff+strlen(headerword));
          rc=getFTinfoFromRecord(buff);
          if(rc){printf("%s%d \n",msg[63],__LINE__);fclose(fp);return -2;}
          sprintf(path,".familytrees/%s",ftn);
          if(!access(path,0)){
             printf("%s %s:",ftn,msg[84]);
             scanfStr(ync,(sizeof(ync)-1));
             trimStr(ync);
             if(strcmp(ync,"y\0")) {fclose(fp);return -3;};
             printf("%s:",msg[91]);
             do{
                memset(ftn,0,sizeof(ftn));
                scanfStr(ftn,sizeof(ftn)-1);
                trimStr(ftn);
                if('\0'==ftn[0]) {printf("%s:",msg[97]);continue;}
                if(isSpaceInString(ftn,strlen(ftn))) {memset(ftn,0,sizeof(ftn));printf("%s:",msg[101]);continue;}
                if(isIncludeControlChars(ftn)) {memset(ftn,0,sizeof(ftn));printf("%s:",msg[96]);continue;}
                sprintf(path,".familytrees/%s",ftn);
                if(!access(path,0)) printf("%s %s:",ftn,msg[16]);
             }while(!access(path,0));

             namechangeflag=1;
          }
      }else{
          strcat(buff,"\n");
          per=getPersonFromRecord(buff);
          if(NULL==per){printf("%s:%d\n",msg[62],__LINE__);return -7; }
          if(g_id<=per->id) g_id=per->id+1;

          if(strstr(buff,"--")){
              if(addChild(per->father,per)){printf("%s:%d\n",msg[63],__LINE__);fclose(fp);return -5;}
           }else{
              if(g_tree_root){
                  printf("%s.\n",msg[105]);
                  initSystemData();
                  fclose(fp);
                  return -6;
              }
              g_tree_root=per;
           }
      }
   }

   fclose(fp);
   writeFamilyTreeToSystemFile();

   FILE *fpl = fopen(".familytrees/.familytreeslist.ftl","a");
   if(fpl==NULL){printf("%s:%d\n",msg[62],__LINE__);free(fullstr);return -4;}
   char now_date[DATE_LEN]={0};
   getNowDateStr(now_date,DATE_LEN,1);
   if(namechangeflag) fprintf(fpl,"%s %s #@%s:%s\n",ftn,now_date,msg[64],ftinfo);
   else fprintf(fpl,"%s\n",ftinfo);
   fclose(fpl);
   free(fullstr);

   printf("%s\n",msg[106]);

   return 0;
}


int readFamilyTreeFromSystemFile()
{
   FILE *fp = NULL;
   char *p;
   char buff[BUFF_LEN]={0};
   PersonInLaw_t ** p_perIL;
   PersonInLaw_t * perIL;
   Person_t * per;
   int i;
   char defaultpath[PATH_LEN]={0};

   printf("%s:",msg[15]);
   memset(ftn,0,sizeof(ftn));
   scanfStr(ftn,sizeof(ftn)-1);
   trimStr(ftn);
   if('\0'==ftn[0]) {printf("%s\n",msg[97]);return -1;}
   if(isSpaceInString(ftn,strlen(ftn))) {printf("%s\n",msg[101]);return -1;}

   sprintf(defaultpath,".familytrees/%s/data.ftd",ftn);
   fp = fopen(defaultpath, "r");
   if(fp==NULL){printf("%s %s\n",ftn,msg[17]);return -1;}
   while(fgets(buff, BUFF_LEN, (FILE*)fp)!=NULL){
      per=getPersonFromRecord(buff);
      if(NULL==per){printf("%s:%d\n",msg[62],__LINE__);fclose(fp);return -3; }

      if(g_id<=per->id) g_id=per->id+1;

      if(strstr(buff,"--")){
          if(addChild(per->father,per)){printf("%s:%d\n",msg[63],__LINE__);fclose(fp);return -1;}
      }else{
          if(g_tree_root){
              printf("%s.\n",msg[105]);
              initSystemData();
              fclose(fp);
              return -2;
          }
          g_tree_root=per;
      }
 
   }
   fclose(fp);

   fp = fopen(".familytrees/.familytreeslist.ftl", "r");
   if(fp==NULL){printf("%s \n",msg[49]);return -2;}
   while(fgets(buff, BUFF_LEN, (FILE*)fp)!=NULL){
      if(isTheLine(buff,ftn)){
         if((p=strstr(buff,"@#"))){
            p=p+2;
            strcpy(ft_note,p);
         }
      }
   }
  
   printf("%s\n",msg[43]);

   return 0;
}

void displayFamilyTreesList(){
   char buff[BUFF_LEN]={0};
   FILE *fp = fopen(".familytrees/.familytreeslist.ftl", "r");
   if(fp==NULL){printf("%s \n",msg[49]);return;}
   printf("%s:\n",msg[35]);
   while(fgets(buff, BUFF_LEN, (FILE*)fp)!=NULL){
      printf("%s",buff);
   }
}

int renameFamilyTree(char * ft_ori_name,char * ft_new_name)
{
   char path_ori[PATH_LEN]={0};
   char path_new[PATH_LEN]={0};
   if(access(".familytrees",0)){
      mkdir(".familytrees",0777);
   }
   sprintf(path_ori,".familytrees/%s",ft_ori_name);
   if(access(path_ori,0)){
      printf("%s %s\n",ft_ori_name,msg[17]);
      return -1; 
   }

   sprintf(path_new,".familytrees/%s",ft_new_name);
   if(!access(path_new,0)){
     printf("%s %s",ft_new_name,msg[16]);
     return -2;
   }

   rename(path_ori,path_new);

   char buff[BUFF_LEN];
   char now_date[DATE_LEN]={0};
   FILE *fp = fopen(".familytrees/.familytreeslist.ftl", "r");
   if(fp==NULL){printf("%s:%d\n",msg[62],__LINE__);return -100;}
   FILE *fp_tmp=fopen(".familytrees/.familytreeslisttemp","w");
   while(fgets(buff, BUFF_LEN, (FILE*)fp)!=NULL){
      if(isTheLine(buff,ft_ori_name)){
         getNowDateStr(now_date,DATE_LEN,1);
         fprintf(fp_tmp,"%s %s #@%s:%s",ft_new_name,now_date,msg[64],buff);
      }else{
         fprintf(fp_tmp,"%s",buff);
      }
   }
 
   remove(".familytrees/.familytreeslist.ftl");
   rename(".familytrees/.familytreeslisttemp",".familytrees/.familytreeslist.ftl");
   fclose(fp);
   fclose(fp_tmp);

   return 0;
}

int updateFamilyTreeNote(char *name,char *note)
{
   char *p;
   char path[PATH_LEN]={0};
   char content[BUFF_LEN*2]={0};
   if(access(".familytrees",0)){
      mkdir(".familytrees",0777);
   }
   sprintf(path,".familytrees/%s",name);
   if(access(path,0)){
      printf("%s %s\n",name,msg[17]);
      return -1;
   }

   char buff[BUFF_LEN];
   FILE *fp = fopen(".familytrees/.familytreeslist.ftl", "r");
   if(fp==NULL){printf("%s:%d\n",msg[62],__LINE__);return -100;}
   FILE *fp_tmp=fopen(".familytrees/.familytreeslisttempabdsldksk_039201ks","w");
   while(fgets(buff, BUFF_LEN, (FILE*)fp)!=NULL){
      if(isTheLine(buff,name)){
          if((p=strstr(buff,"@#"))) *p='\0';
          if((p=strchr(buff,'\n'))) *p='\0';
          strcpy(content,buff);
          strcat(content,"@#");
          strcat(content,note); 
 
          fprintf(fp_tmp,"%s\n",content);
      }else{
         fprintf(fp_tmp,"%s",buff);
      }
   }

   remove(".familytrees/.familytreeslist.ftl");
   rename(".familytrees/.familytreeslisttempabdsldksk_039201ks",".familytrees/.familytreeslist.ftl");
   fclose(fp);
   fclose(fp_tmp);

   return 0;

}

int deleteFamilyTree(char * ft_name)
{
   char path_src[PATH_LEN]={0};
   char path_des[PATH_LEN]={0};
   if(access(".familytrees/.rmv",0)){
      mkdir(".familytrees/.rmv",0777);
   }

   char now_date[DATE_LEN]={0};
   getNowDateStr(now_date,DATE_LEN,1);
   FILE *fp = fopen(".familytrees/.rmv/.deletefamilytreeslist.ftl","a");
   if(fp==NULL){printf("%s:%d\n",msg[62],__LINE__);return -100;}
   fprintf(fp,"%s\tdeleted time:%s\n",ft_name,now_date);
   fclose(fp);

   sprintf(path_src,".familytrees/%s",ft_name);
   *(strchr(now_date,' '))='_';
   sprintf(path_des,".familytrees/.rmv/%s__%s",ft_name,now_date);
   rename(path_src,path_des);

   time_t now;
   time(&now);
   char buff[BUFF_LEN];
   char name_tmp[NAME_LEN]={0};
   sprintf(name_tmp,".familytrees/.familytreeslisttempabdsldksk_039201ksyew%ld",now);
   fp = fopen(".familytrees/.familytreeslist.ftl", "r");
   if(fp==NULL){printf("%s:%d\n",msg[62],__LINE__);return -100;}
   FILE *fp_tmp=fopen(name_tmp,"w");
   while(fgets(buff, BUFF_LEN, (FILE*)fp)!=NULL){
      if(!isTheLine(buff,ft_name)){
         fprintf(fp_tmp,"%s",buff);
      }
   }

   remove(".familytrees/.familytreeslist.ftl");
   rename(name_tmp,".familytrees/.familytreeslist.ftl");
   fclose(fp);
   fclose(fp_tmp);

   return 0;
}

void getIDsByName(char * name,Person_t * searchTree,ulong search_id[],int scope){
  if(NULL==searchTree) return;
  int i;
  if((scope==ALL_TYPE)||(scope==PERSON_TYPE)){
      if(0==strcmp(name,searchTree->name)){
         if(search_id[0]>=MAX_DUPNAME) {printf("%s:%d",msg[50],__LINE__);return;}
         search_id[++search_id[0]]=searchTree->id;
      }
  }
  if((scope==ALL_TYPE)||(scope==PERSONINLAW_TYPE)){
      if(searchTree->n_spouse){
         for(i=0;i<searchTree->n_spouse;i++){
            if(0==strcmp(name,searchTree->spouse[i]->name)){
                if(search_id[0]>=MAX_DUPNAME) {printf("%s:%d",msg[50],__LINE__);return;}
                search_id[++search_id[0]]=searchTree->spouse[i]->id;
            }
         }
      }
  }
  if(searchTree->n_childreninclan!=0){
       for(i=0;i<searchTree->n_childreninclan;i++)
       {
          getIDsByName(name,searchTree->childreninclan[i],search_id,scope);
       }
   }

   return;
}

int isDupOfName(char *name,Person_t * searchTree,int scope){
   ulong search_id[MAX_DUPNAME+1]={0};
   getIDsByName(name,searchTree,search_id,scope);
   if(search_id[0]>1) return 1; 
   return 0;
}

void printFamilyChainForSomeone(Person_t * per){
   int i,j;
   Person_t *p;
   if(per->father)printFamilyChainForSomeone(per->father);
   
   per->n_childreninclan_print=per->n_childreninclan;
   for(i=1;i<per->generation;i++){
      p = per;
      for(j=per->generation;j>i;j--){
          p=p->father;
      }
      printf("     ");
   }
   if(per->father) per->father->n_childreninclan_print--;

   if(1==per->generation)printf("___%s(%s%lu)",per->name,msg[58],per->generation);
   else printf("|___%s(%s%lu)",per->name,msg[58],per->generation);
   printf("\n");
}


void getNumOfPerson(Person_t *node,int num[][2])
{
    if(node->sex==1)num[node->generation][0]++;
    else num[node->generation][1]++;
    for(int i=0;i<node->n_childreninclan;i++)
    {
       getNumOfPerson(node->childreninclan[i],num);
    }
    return;
}

void printStatistics(Person_t* node)
{
    if(node==NULL){printf("%s:%d",msg[61],__LINE__);return;}
    int numofper[MAX_GEN][2]={0};
    printf("%s: %s\n",msg[80],ftn); 
    getNumOfPerson(node,numofper);
    for(int i=0;i<MAX_GEN;i++){
       if(numofper[i][0] || numofper[i][1]) printf("%s %02d：%s %02d，%s %02d\n",msg[58],i,msg[99],numofper[i][0],msg[100],numofper[i][1]);
    }
    return;
}


void updateRelativeGenerationInternel(Person_t *node)
{
    if(node->father){
       if(node->father->relative_generation) node->relative_generation=node->father->relative_generation+1;
       else node->relative_generation=0;
    }
    for(int i=0;i<node->n_childreninclan;i++)
    {
       updateRelativeGenerationInternel(node->childreninclan[i]);
    }  

}

void updateRelativeGeneration(ulong gen)
{
    if(g_tree_root==NULL){printf("%s\n",msg[48]);}
    g_tree_root->relative_generation=gen;
    updateRelativeGenerationInternel(g_tree_root);
}

Person_t * getPersonByInputNameInteractively(int msgid){
    printf("%s:",msg[msgid]);
    char name[NAME_LEN]={0};
    char IDstr[ID_LEN]={0};
    ulong id;
    scanfStr(name,(sizeof(name)-1));
    trimStr(name);
    Person_t * per;
    if(isDupOfName(name,g_tree_root,PERSON_TYPE)){
        printf("%s %s:",name,msg[24]);
        scanfStr(IDstr,(sizeof(IDstr)-1));
        trimStr(IDstr);
        if(0==strcmp(IDstr,"q")) return NULL;
        if(isAllDigits(IDstr)) id=atoi(IDstr);
        else{ printf("%s\n",msg[75]); return NULL; }
        per=getPersonByID(id,g_tree_root);
        if(per==NULL){
           printf("ID:%ld %s\n",id,msg[20]);
           return NULL;
        }
        if(strcmp(per->name,name)){
           printf("%lu:%s %s\n",id,per->name,msg[95]);
           return NULL;
        }
    }else{
        per=getPersonByName(name,g_tree_root);
        if(per==NULL){
           printf("%s %s\n",name,msg[19]);
           return NULL;
        }
    }

    return per;
}

PersonInLaw_t * getPersonInLawByInputNameInteractively(int msgid){
    printf("%s:",msg[msgid]);
    char name[NAME_LEN]={0};
    char IDstr[ID_LEN]={0};
    ulong id;
    scanfStr(name,(sizeof(name)-1));
    trimStr(name);
    PersonInLaw_t * perIL;

    if(isDupOfName(name,g_tree_root,PERSONINLAW_TYPE)){
        printf("%s %s:",name,msg[24]);
        scanfStr(IDstr,(sizeof(IDstr)-1));
        trimStr(IDstr);
        if(0==strcmp(IDstr,"q")) return NULL;
        if(isAllDigits(IDstr)) id=atoi(IDstr);
        else{ printf("%s\n",msg[75]); return NULL; }
        perIL=getSpouseByID(id,g_tree_root);
        if(perIL==NULL){
           printf("ID:%ld %s\n",id,msg[20]);
           return NULL;
        }
        if(strcmp(perIL->name,name)){
           printf("%lu:%s %s\n",id,perIL->name,msg[95]);
           return NULL;
        }
    }else{
        perIL=getSpouseByName(name,g_tree_root);
        if(perIL==NULL){
           printf("%s %s\n",name,msg[19]);
           return NULL;
        }
    }

    return perIL;
}

Person_t * newPersonByInputNameInteractively(int msgid){
    printf("%s:",msg[msgid]);
    char ync[8]={0};
    char sex[8]={0};
    char order[8]={0};
    char name[NAME_LEN]={0};

    scanfStr(name,(sizeof(name)-1));
    trimStr(name);
    if(isIncludeControlChars(name)) {printf("%s\n",msg[96]);return NULL;}

    Person_t *per=(Person_t *)malloc(sizeof(Person_t));
    memset(per,0,sizeof(Person_t));
    strcpy(per->name,name);
    printf("%s:",msg[27]);
    memset(sex,0,sizeof(sex));
    scanfStr(sex,(sizeof(sex)-1));
    trimStr(sex);
    if(strcmp(sex,"0")&&strcmp(sex,"1")){
        free(per);
        per=NULL;
        printf("%s\n",msg[74]);
        return NULL;
    }
    per->sex=atoi(sex);

    printf("%s:",msg[28]);
    memset(order,0,sizeof(order));
    scanfStr(order,(sizeof(order)-1));
    trimStr(order);
    if(isAllDigits(order)){
        per->order=atoi(order);
    }else{
        free(per);
        per=NULL;
        printf("%s\n",msg[75]);
        return NULL;
    }

    per->id=getID();
    strcpy(per->up_ft,"NULL");
    strcpy(per->down_ft,"NULL");
    return per;
}

int processEditingFTInteractively(){
    if(NULL == g_tree_root) return 0;
    char ync[NAME_LEN]={0};
    printf("%s:",msg[76]);
    scanf("%s",ync);
    if(strcmp(ync,"y\0")) return -1; 

    printf("%s:",msg[77]);
    memset(ync,0,sizeof(ync));
    scanf("%s",ync);
    if(strcmp(ync,"n\0")) {
        writeFamilyTreeToSystemFile();
        printf("%s %s\n",msg[78],ftn);
    }else{
        printf("%s %s\n",msg[79],ftn);
    }

    deleteSubTree(g_tree_root);
    if(g_tree_root)free(g_tree_root);
    initSystemData();
    return 0;
}

int moveOrder(Person_t * per, int direction)
{
    if(NULL==per) {printf("%s\n",msg[9]);return -1;}
    if(NULL==per->father) {printf("%s\n",msg[104]);return -2;}
    int num=per->father->n_childreninclan;
    if(num<=1) {printf("%s\n",msg[104]);return -3;}
    Person_t *per_tmp;
    int i = 0;
    if(UP==direction){
        if(per->father->childreninclan[0]->id==per->id) {printf("%s\n",msg[104]);return -4;}
        for(i=1;i<num;i++)
            if(per->father->childreninclan[i]->id==per->id) break;
        per_tmp=per->father->childreninclan[i-1];
        per->father->childreninclan[i-1]=per;
        per->father->childreninclan[i]=per_tmp;
    }    
    if(DOWN==direction){
        if(per->father->childreninclan[num-1]->id==per->id) {printf("%s\n",msg[104]);return -5;}
        for(i=0;i<num-1;i++)
            if(per->father->childreninclan[i]->id==per->id) break;
        per_tmp=per->father->childreninclan[i+1];
        per->father->childreninclan[i+1]=per;
        per->father->childreninclan[i]=per_tmp;
    }
    return 0;
}

int main()
{
  char option[NAME_LEN]={0};
  char name[NAME_LEN]={0};
  char name_tmp[NAME_LEN]={0};
  char path[PATH_LEN]={0};
  char note[BUFF_LEN]={0};
  char intstr[BUFF_LEN]={0};
  char keystr[MAX_KEY+1]={0};
  char keystr2[MAX_KEY+1]={0};
  char ync[NAME_LEN]={0};
  Person_t *per=NULL;
  Person_t *per2=NULL;
  Person_t *per3=NULL;
  PersonInLaw_t *perIL=NULL;
  PersonInLaw_t *perIL2=NULL;
  ulong id=0;
  ulong gen=0;
  int i=0;
  printf("默认语言：简体中文(按回车键继续)。   Enter 1 for English. :");
  scanf("%2[^\n]",option);
  if(0==strncmp(option,"1\0",2)) {help=help_en;msg=msg_en;} 
  printf("%s\n",msg[66]);
  while(1){
       printf(">>");
       memset(option,0,sizeof(option));
       fflush(stdin);
       fflush(stdout);
       scanfStr(option,(sizeof(option)-1));
       trimStr(option);
       i=0;
       memset(name,0,sizeof(name));
       memset(name_tmp,0,sizeof(name_tmp));
       memset(path,0,sizeof(path));
       memset(note,0,sizeof(note));
       memset(ync,0,sizeof(ync));
       memset(intstr,0,sizeof(intstr));
       if(strcmp(option,"createfamilytree")==0||strcmp(option,"cft")==0||strcmp(option,"c")==0){
              if(processEditingFTInteractively()) continue;
              createFamilyTree();
       }
       else if(strcmp(option,"addchild")==0||strcmp(option,"ac")==0){
               per=getPersonByInputNameInteractively(18);
               if(NULL==per) continue;
               per2=newPersonByInputNameInteractively(21);
               if(NULL==per2) continue;
               addChild(per,per2);
       }
       else if(strcmp(option,"addspouse")==0||strcmp(option,"as")==0){
               per=getPersonByInputNameInteractively(23);
               if(NULL==per) continue;

               perIL=(PersonInLaw_t *)malloc(sizeof(PersonInLaw_t));
               memset(perIL,0,sizeof(PersonInLaw_t));
               printf("%s:",msg[25]);
               scanfStr(name,(sizeof(name)-1));
               trimStr(name);
               if(isIncludeControlChars(name)) {printf("%s\n",msg[96]);return -1;}
               strcpy(perIL->name,name);

               printf("%s:",msg[27]);
               memset(intstr,0,sizeof(intstr));
               scanf("%2s",intstr);
               if(strcmp(intstr,"0")&&strcmp(intstr,"1")){
                  free(perIL);
                  perIL=NULL;
                  printf("%s\n",msg[74]);
                  continue;
               }
               perIL->sex=atoi(intstr);
               perIL->id=getID();
               addSpouse(per,perIL);
       }
       else if(strcmp(option,"updatename")==0||strcmp(option,"un")==0){      
               per=getPersonByInputNameInteractively(29);
               if(NULL==per) continue; 
               printf("%s:",msg[30]);
               scanfStr(name,(sizeof(name)-1));
               trimStr(name);
               if(isIncludeControlChars(name)) {printf("%s\n",msg[96]);continue;}

               strcpy(per->name,name);
       }
       else if(strcmp(option,"updatesex")==0||strcmp(option,"us")==0){ 
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;

               printf("%s:",msg[27]);
               memset(intstr,0,sizeof(intstr));
               scanf("%2s",intstr);
               if(strcmp(intstr,"0")&&strcmp(intstr,"1")){
                   printf("%s\n",msg[74]);
                   continue;
               }
               per->sex=atoi(intstr);
       }
       else if(strcmp(option,"updatenote")==0||strcmp(option,"unt")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printf("%s:",msg[54]);

               scanfStr(note,(sizeof(note)-1));
               trimStr(note);
               if(isIncludeControlChars(note)) {printf("%s\n",msg[96]);continue;;}
               strcpy(per->note,note);
       }
       else if(strcmp(option,"updateorder")==0||strcmp(option,"uo")==0){ 
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printf("%s:",msg[28]);
               memset(intstr,0,sizeof(intstr));
               scanf("%16s",intstr);
               if(isAllDigits(intstr)){
                   per->order=atoi(intstr);
               }else{
                   printf("%s\n",msg[75]);
                   continue;
               }
       }
       else if(strcmp(option,"moveup")==0||strcmp(option,"mu")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               moveOrder(per,UP);
       }
       else if(strcmp(option,"movedown")==0||strcmp(option,"md")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               moveOrder(per,DOWN);
       }
       else if(strcmp(option,"updateupft")==0||strcmp(option,"uu")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printf("%s:",msg[71]);
               scanfStr(name,sizeof(name)-1);
               trimStr(name);
               if(isSpaceInString(name,strlen(name))) {printf("%s\n",msg[101]);continue;}
               if(isIncludeControlChars(name)) {printf("%s\n",msg[96]);continue;}
               strcpy(per->up_ft,name);
       }
       else if(strcmp(option,"updatedownft")==0||strcmp(option,"ud")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printf("%s:",msg[72]);
               scanfStr(name,sizeof(name)-1);
               trimStr(name);
               if(isSpaceInString(name,strlen(name))) {printf("%s\n",msg[101]);continue;}
               if(isIncludeControlChars(name)) {printf("%s\n",msg[96]);continue;}
               strcpy(per->down_ft,name);
       }
      else if(strcmp(option,"updaterelativegeneration")==0||strcmp(option,"ur")==0){
               if(g_tree_root==NULL){printf("%s\n",msg[48]);continue;}
               printf("%s:",msg[67]);
               scanf("%16s",intstr);
               if(isAllDigits(intstr)){
                   gen=atoi(intstr);
               }else{
                   printf("%s\n",msg[75]);
                   continue;
               }
               updateRelativeGeneration(gen);
               
       }
       else if(strcmp(option,"printfamilysimplest")==0||strcmp(option,"pfs")==0){
               printFamilyTree(g_tree_root,PRINTNAMEONLY);
       }
       else if(strcmp(option,"printfamilysimple")==0||strcmp(option,"p")==0){ 
               printFamilyTree(g_tree_root,PRINTNAMENOTE);
       }
       else if(strcmp(option,"printfamily")==0||strcmp(option,"pf")==0){ 
               printFamilyTree(g_tree_root,PRINTNAMESEX);
       }
       else if(strcmp(option,"printfamilydetails")==0||strcmp(option,"pfd")==0||strcmp(option,"d")==0){
               printFamilyTree(g_tree_root,PRINTALL);
       }
       else if(strcmp(option,"printfamilyhtml")==0||strcmp(option,"pfh")==0){
               printFamilyTreeStyleToHtmlFile();
       }
       else if(strcmp(option,"printsubtreehtml")==0||strcmp(option,"psh")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printf("%s:",msg[56]);
               scanf("%s",name_tmp);
               printSubtreeToHtmlFile(name_tmp,per);
       }
       else if(strcmp(option,"printsubtreesimple")==0||strcmp(option,"pss")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printFamilyTree(per,PRINTNAMENOTE);
       }
       else if(strcmp(option,"printchain")==0||strcmp(option,"pc")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printf("%s:\n",msg[59]);
               printFamilyChainForSomeone(per);
       }
       else if(strcmp(option,"printspousebyID")==0||strcmp(option,"psi")==0){
               printf("%s:",msg[38]);
               scanf("%s",intstr);
               if(isAllDigits(intstr)){
                   id=atoi(intstr);
               }else{
                   printf("%s\n",msg[75]);
                   continue;
               }
               perIL=getSpouseByID(id,g_tree_root);
               if(perIL==NULL){
                   printf("ID:%ld %s\n",id,msg[20]);
                   continue;
               }
               printPersonInLaw(perIL);
       }
       else if(strcmp(option,"printspouse")==0||strcmp(option,"ps")==0){
               perIL=getPersonInLawByInputNameInteractively(31);
               if(NULL==perIL) continue;
               printPersonInLaw(perIL);
       }
       else if(strcmp(option,"printpersonbyID")==0||strcmp(option,"pi")==0){
               printf("%s:",msg[38]);
               scanf("%s",intstr);
               if(isAllDigits(intstr)){
                   id=atoi(intstr);
               }else{
                   printf("%s\n",msg[75]);
                   continue;
               }  
               per=getPersonByID(id,g_tree_root);
               if(per==NULL){
                   printf("ID:%ld %s\n",id,msg[20]);
                   continue;
               }
               printPerson(per);
       }
       else if(strcmp(option,"printperson")==0||strcmp(option,"pp")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printPerson(per);
       }
       else if(strcmp(option,"help")==0||strcmp(option,"h")==0){
               while(strcmp(help[i][0],"\0")){
                     printf("%-64s%-s\n",help[i][0],help[i][1]);
                     i++;
               }
       }
       else if(strcmp(option,"serialize")==0||strcmp(option,"ser")==0){
              if(g_tree_root==NULL){printf("%s\n",msg[48]);continue;}
              serializeFamilyTreeToFile(g_tree_root,NULL); 
       }
       else if(strcmp(option,"readfamilytree")==0||strcmp(option,"rft")==0||strcmp(option,"r")==0){
              if(processEditingFTInteractively()) continue;
              readFamilyTreeFromSystemFile();
       }
       else if(strcmp(option,"savefamilytree")==0||strcmp(option,"wft")==0||strcmp(option,"s")==0){
              writeFamilyTreeToSystemFile();
       }
       else if(strcmp(option,"listfamilytrees")==0||strcmp(option,"list")==0||strcmp(option,"l")==0){
              displayFamilyTreesList();
       }
       else if(strcmp(option,"deletefamilytree")==0||strcmp(option,"df")==0){
              printf("%s:",msg[32]);
              scanfStr(name,sizeof(name)-1);
              trimStr(name);
              if('\0'==name[0]) {printf("%s\n",msg[97]);continue;}
              if(isSpaceInString(name,strlen(name))) {printf("%s\n",msg[101]);continue;}
              sprintf(path,".familytrees/%s",name);
              if(access(path,0)){
                 printf("%s %s\n",name,msg[17]);
                 continue;
              }
              if(strcmp(name,ftn)==0){
                 printf("%s %s:",name,msg[98]);
                 scanf("%s",ync);
                 if(strcmp(ync,"y")) continue;
                 deleteSubTree(g_tree_root);
                 if(g_tree_root)free(g_tree_root);
                 initSystemData();
              }
              deleteFamilyTree(name);
       }
       else if(strcmp(option,"renamefamilytree")==0||strcmp(option,"rf")==0){
              printf("%s:",msg[33]);
              scanfStr(name,sizeof(name)-1);
              trimStr(name);
              if('\0'==name[0]) {printf("%s\n",msg[97]);continue;}
              if(isSpaceInString(name,strlen(name))) {printf("%s\n",msg[101]);continue;}
              sprintf(path,".familytrees/%s",name);
              if(access(path,0)){
                 printf("%s %s\n",name,msg[17]);
                 continue;
              }
              memset(path,0,sizeof(path));
              printf("%s:",msg[30]);
              scanfStr(name_tmp,sizeof(name_tmp)-1);
              trimStr(name_tmp);
              if(isSpaceInString(name_tmp,strlen(name_tmp))) {printf("%s\n",msg[101]);return -1;}
              if(isIncludeControlChars(name_tmp)) {printf("%s\n",msg[96]);return -1;}
              sprintf(path,".familytrees/%s",name_tmp);
              if(!access(path,0)){
                 printf("%s %s\n",name_tmp,msg[16]);
                 continue;
              }

              renameFamilyTree(name,name_tmp);
              if(strcmp(name,ftn)==0){
                 memset(ftn,0,sizeof(ftn));
                 strcpy(ftn,name_tmp);  
              }
       }
       else if(strcmp(option,"updatenoteoffamilytree")==0||strcmp(option,"unf")==0){
              printf("%s:",msg[33]);
              scanfStr(name,sizeof(name)-1);
              trimStr(name);
              if('\0'==name[0]) {printf("%s\n",msg[97]);continue;}
              if(isSpaceInString(name,strlen(name))) {printf("%s\n",msg[101]);continue;}
              sprintf(path,".familytrees/%s",name);
              if(access(path,0)){
                 printf("%s %s\n",name,msg[17]);
                 continue;
              }
              printf("%s:",msg[54]);
              scanfStr(note,(sizeof(note)-1));
              trimStr(note);
              if(isIncludeControlChars(note)) {printf("%s\n",msg[96]);;continue;}

              updateFamilyTreeNote(name,note);
              if(strcmp(name,ftn)==0){
                 memset(ft_note,0,sizeof(ft_note));
                 strcpy(ft_note,note);
              }
       }
       else if(strcmp(option,"deletesubtree")==0||strcmp(option,"dst")==0){
               per=getPersonByInputNameInteractively(41);
               if(NULL==per) continue;
               if(deleteSubTree(per)){printf("%s\n",msg[39]);}else{printf("%s\n",msg[40]);};
       }
       else if(strcmp(option,"searchbyname")==0||strcmp(option,"sbn")==0){
               printf("%s:",msg[31]);
               scanfStr(name,(sizeof(name)-1));
               trimStr(name);
               ulong search_id[MAX_DUPNAME+1]={0}; //search[0] restore the number of valid id.
               getIDsByName(name,g_tree_root,search_id,PERSON_TYPE);
   	       if(search_id[0]>0){
                   printf("%s:",msg[52]);
                   for(i=1;i<=search_id[0];i++){
                       per=getPersonByID(search_id[i],g_tree_root);
                       if(per->father)printf("%lu(%s %s) ",search_id[i],msg[34],per->father->name);
		       else printf("%lu(%s) ",search_id[i],msg[103]);
                   }
               }
               ulong search_id2[MAX_DUPNAME+1]={0}; //search[0] restore the number of valid id.
               getIDsByName(name,g_tree_root,search_id2,PERSONINLAW_TYPE);
               if(search_id2[0]>0){
                  printf("\n%s:",msg[53]);
                  for(i=1;i<=search_id2[0];i++){
                     perIL=getSpouseByID(search_id2[i],g_tree_root);
                     printf("%lu(%s %s) ",search_id2[i],msg[11],perIL->relation->name);
                  }
               }
               if(0==(search_id[0]+search_id2[0])){printf("%s:%s\n",name,msg[51]);continue;}
               printf("\n");
       }
       else if(strcmp(option,"statistics")==0||strcmp(option,"st")==0){
              if(g_tree_root==NULL){printf("%s\n",msg[48]);continue;}
              printStatistics(g_tree_root); 
       }
       else if(strcmp(option,"statisticsofsubtree")==0||strcmp(option,"ss")==0){
               per=getPersonByInputNameInteractively(31);
               if(NULL==per) continue;
               printStatistics(per);
       }
       else if(strcmp(option,"exportfamilytreetofile")==0||strcmp(option,"eft")==0){
               printf("%s:",msg[15]);
               scanf("%s",name);
               printf("%s:",msg[83]);
               scanf("%s",name_tmp);
               printf("%s:",msg[87]);
               memset(keystr,0,sizeof(keystr));
               getPass(keystr,(sizeof(keystr)-1));
               if(strlen(keystr)>MAX_KEY||strlen(keystr)<MIN_KEY){printf("\n%s\n",msg[90]);continue;}
               memset(keystr2,0,sizeof(keystr2));
               printf("\n%s:",msg[93]);
               getPass(keystr2,(sizeof(keystr2)-1));
               if(strcmp(keystr,keystr2)){printf("\n%s\n",msg[94]);continue;}
               if(0==strcmp(ftn,name)){
                   printf("\n%s:",msg[82]);
                   scanf("%s",ync);
                   if(strcmp(ync,"y")) continue;    
                   writeFamilyTreeToSystemFile();
               }
               putchar('\n');
               exportFamilyTreeToFile(name,name_tmp,keystr);
       }
       else if(strcmp(option,"importfamilytreefromfile")==0||strcmp(option,"ift")==0){
               if(processEditingFTInteractively()) continue;
               printf("%s:",msg[85]);
               scanf("%s",path);
               printf("%s:",msg[89]);
               memset(keystr,0,sizeof(keystr));
               getPass(keystr,MAX_KEY);
               putchar('\n');
               importFamilyTreeFromFile(path,keystr);
       }
       else if(strcmp(option,"quit")==0 || strcmp(option,"q")==0 ){
               if(g_tree_root){
                   printf("%s:",msg[77]);
                   scanf("%s",ync);
                   if(strcmp(ync,"n")) writeFamilyTreeToSystemFile();
               }
               return 0;
       }
       else {
              printf("%s: '%s'\n",msg[47],option);     
       } 
    }
    return 0; 
}
