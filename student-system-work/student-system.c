/**
 * @file student-system.cpp
 * @brief 课堂设计: 成绩管理系统
 * @details 基础的成绩管理系统，按照要求设计。 \n
 *   请在编译时使用 `-fexec-charset=UTF-8` 参数以正常显示中文。
 * @attention This file MUST BE OPENED (ALSO SAVED) IN `UTF-8`, or it won't work/show correctly. \n
 *   In windows, run `chcp 65001` first before compiling. \n
 *   Add `-fexec-charset=UTF-8` argrument when compiling it.
 * 
 * @author JoyWonderful
 * @date 2025-12
 */
#include <locale.h>
#include <string.h>
#include <stdio.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h> // 用来设置代码页为 65001；正常显示 ANSI 转义序列。见 `main_env_setup()`
#endif

// If the color displays correctly, don't define DISABLE_STYLE.


////////////////////////////////
/// Macros & Structure define //
////////////////////////////////

// Begin Style ///////////////
// 参见 https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#text-formatting
// Windows 参见 https://learn.microsoft.com/en-us/windows/console/classic-vs-vt
#ifndef DISABLE_STYLE
#define STYLE_RESET_ALL "\033[0m"
#define STYLE_HIGHLIGHT "\033[1m"
#define STYLE_DIM "\033[2m"
#define STYLE_ITALIC "\033[3m"
#define STYLE_LIGHT_BLUE "\033[94m"
#define STYLE_LIGHT_CYAN "\033[96m"
#define OUTPUT_HR_LINE "===================="
#define OUTPUT_WARN "\033[33mWarning: "
#define OUTPUT_ERR "\033[31mError: "
#else
#define STYLE_RESET_ALL ""
#define STYLE_HIGHLIGHT ""
#define STYLE_DIM ""
#define STYLE_ITALIC ""
#define STYLE_LIGHT_BLUE ""
#define STYLE_LIGHT_CYAN ""
#define OUTPUT_HR_LINE ""
#define OUTPUT_WARN ""
#define OUTPUT_ERR ""
#endif
// End Style /////////////////
typedef unsigned short USH; // unsigned short / 表示完成度，通常在 0~9 之间 / 表示布尔

#define MAXN 1000 // 最大学生数
#define MAXCLASS 50 // 最大班级数
#define MAXSTUID (int)1e6 // 最大学生学号
#define MAXCLASSID (int)1e4 // 最大班级编号
/*const*/ char SCORE_TABLE[5][15] = {"高数", "英语", "体育", "线代", "程序设计"};
struct Student {// 学生结构体
    int id; // 学号
    char name[20]; // 姓名
    int cla; // 所属班级
    int clark; // 班级内在 `Student *members` 所处下标
    double score[5]; // 各科分数
    double totscore; // 总分
} stu[MAXN]; // 总数据库
struct SchClass {// 班级结构体
    int id; // 那个班级
    int memcnt; // 班级内学生个数
    struct Student *members[MAXN]; // 班级内学生
    double totscore; // 总分
    double average; // 总分的平均分
} stuclass[MAXCLASS]; // 班级数据库
struct SchClass *classid[MAXCLASSID]; // 班级指针数组,[class]，指向 `stuclass`
struct Student *stuid[MAXSTUID]; // 学号指针数组 [id]，指向 `stu`
int stucnt; // 学生总个数
int classcnt; // 班级总个数

///////////////////////////////////
/// Functions that format output //
///////////////////////////////////

/**
 * @brief 返回应当输出的宽度。 \n
 *   中文字符 => `2` ； \n
 *   英文字符 => `1` 。
 * @details 对于一般的 ASCII 字符，当然也就是英文字符，显示的长度为 `1`，这是 C 内自带函数可以处理的。 \n
 *   对于中文字符，在 UTF-8 中，它的字节长度为 `3`，但是希望显示的长度为 `2`。 \n
 *   因此通过判断字符字节长度来判断显示长度。 \n \n
 *   但是，对于一些有 `3` 字节然而显示长度为 `1` 的字符，它无法正常处理。 \n
 *   就像 `·`（间隔符）。 \n
 *   照理说，正常的输入不应该出现很奇怪的特殊符号，因此就不判断特例了。 \n
 *   ~~为什么非要叫我们用 C 处理中文呢？这适配有多麻烦？用 C++ 不好么？？？~~
 * @param _str 字符串。
 * @return 该字符串显示在终端上的长度，此长度与字符数组的长度无关。
 */
int get_display_width(const char *_str)
{
    int ret_width = 0;
    int len = strlen(_str);
    for (int i = 0; i < len; i++)
    {
        char c = _str[i]; // 一个一个字节地判断
        if ((c & 0xE0) == 0xE0) // 3字节字符，Unicode range:0x0800~0xFFFF (UTF-8: 0b1110XXXX 0b10XXXXXX 0b10XXXXXX)（X 为任意可行数字）
        {   // 即判断是否为UTF-8中文字符的第一个字节 UTF-8 3 字节字符（中文字符，Unicode range:0x4E00~0x9FFF，包含在内）
            // 0b1110XXXX 为 0xE0XX
            ret_width += 2; // 显示宽度为 2
            i += 2;  // 跳过后续两个字节
        } 
        else ret_width += 1; // 普通 ASCII，显示宽度为 1，1 字节
    }
    return ret_width;
}

/**
 * @brief 将字符串缩短到相应的显示长度
 * @param _str 要缩短的字符串（直接在里面进行操作）
 * @param _width 最大长度
 * @return 缩短后字符数组长度。
 */
int shorten_str(char *_str, int _width)
{
    int ret_real_width = 0, display_width = 0;
    int len = strlen(_str);
    int prev_i = 0;
    for(int i = 0; i < len; i++)
    {
        // 这几段同 `get_display_width`。 //
        char c = _str[i];
        if((c & 0xE0) == 0xE0)
        {
            display_width += 2;
            i += 2;
        }
        else display_width += 1;
        ///////////////////////////
        if(display_width > _width)
        {
            _str[prev_i+1] = '\0';
            break;
        }
        prev_i = i;
    }
    return strlen(_str);
}

/**
 * @brief 输出一个 `\033[2m|\033[0m _str` 到 `_res`，_res 肯定是一个字符串地址，形如 `&str[i]` or `str+1`。 \n
 *   它支持合理地**格式化中文**。这是 `printf` 不能够处理的，因此才写这个函数。
 * @param _str 要格式化的字符串
 * @param _res 输出到的位置（直接用到 `sprintf()` 的第一个参数上）
 * @param _width 对齐宽度。相当于 `printf("%4d")` 中的 `4`。
 * @param _align_left `bool`:`0`/`1`。是否居左。 \n
 *   相当于 `printf("%-10d")` 的中的 `-` 格式符。
 * @return 生成字符串的长度（包括换行符与各种转义序列）
 */
int output_my_table_sprintf(/*const*/ char *_str, char *_res, int _width, USH _align_left)
{
    int owidth = get_display_width(_str); // 输出宽度
    int realwidth = strlen(_str); // 真实储存在数组里的宽度
    int padding = _width - owidth; // 要输出的空格（左/右对齐用）
    USH is_too_narrow = 0;
    char *str = _str;
    if(padding < 0) // 要求的不够用了
    {   // 强行截断
        int width_after_shorten = shorten_str(str, _width);
        owidth = get_display_width(str);
        realwidth = width_after_shorten;
        padding = _width - owidth; // 由于中文截断的特殊性，此处应为 0/1 /2?
        is_too_narrow = 1;
    }
    char format_str[1000] = "%s|%s ";
    if(_align_left) // 左对齐
    {
        for(int i = 6; str[i-6] != '\0'; i++) format_str[i] = str[i-6];
        for(int i = 6+realwidth; i < 6+realwidth+padding; i++) format_str[i] = ' ';
    }
    else // 右对齐
    {
        for(int i = 6; i < 6+padding; i++) format_str[i] = ' ';
        for(int i = 6+padding; str[i-6-padding] != '\0'; i++) format_str[i] = str[i-6-padding];
    }
    format_str[6+padding+realwidth] = (is_too_narrow)?'.':' ';
    char res_str[1000];
    sprintf(res_str, format_str, STYLE_DIM, STYLE_RESET_ALL);
    sprintf(_res, format_str, STYLE_DIM, STYLE_RESET_ALL);
    return strlen(res_str);
}

/**
 * @brief “表格”形式输出学生信息。
 * @param _types 0~63 之间，二进制格式化。2^5=`Rank`, 2^4=`ID`, 2^3=`Name`, 2^2=`Class`, 2^1=`Scores`, 2^0=`Total`. \n
 *   e.g 0b011100 会显示 ID, Name, Class
 * @param _stu 数组，包含所有要显示的学生。
 * @param _len 学生个数。
 * @note `_stu` 内排列顺序就是 `Rank` 顺序，即名次顺序。 \n
 *   `Scores` 为各科分数；`Total` 为总分。
 */
void output_table_format(int _types, struct Student *_stu, int _len)
{
    char thead[1000] = {0}, linerow[1000] = {0};
    USH output_type[6] = {0,0,0,0,0,0};// 2^5, 2^4, 2^3, 2^2, 2^1, 2^0
    for(int i = 0; _types > 0; i++) // 获取要输出的类型
    {
        int bit = _types % 2;
        output_type[5-i] = bit; // 十进制转二进制，倒序输出
        _types /= 2;
    }

    // 表格头+分割界限
    int thb_col = 0; // 表格头/体字符串长度，包括换行符、特殊转义序列
    sprintf(linerow, STYLE_DIM);
    int liner_col = 4; // 表格线长度
    /*const*/ char TABLE_HEAD_TYPES[6][20] = {"排行", "#学号", "@姓名", "班级", "", "总分"};
    if(output_type[0]) { // Rank
        //# // 4+1+4+1+4+1
        //# sprintf(thead+thb_col, "%s|%s %4s ", STYLE_DIM, STYLE_RESET_ALL, "Rank");
        int tmp_col = output_my_table_sprintf(TABLE_HEAD_TYPES[0], thead+thb_col, 4, 0);
        thb_col += tmp_col;
        // 1+1+4+1
        //                          | 排行 
        sprintf(linerow+liner_col, "+------");
        liner_col += 7;
    }
    if(output_type[1]) { // ID
        //# // 4+1+4+1+6+1
        //# sprintf(thead+thb_col, "%s|%s %6s ", STYLE_DIM, STYLE_RESET_ALL, "ID");
        int tmp_col = output_my_table_sprintf(TABLE_HEAD_TYPES[1], thead+thb_col, 6, 0);
        thb_col += tmp_col;
        // 1+1+6+1
        //                          | #学号  
        sprintf(linerow+liner_col, "+--------");
        liner_col += 9;
    }
    if(output_type[2]) { // Name
        //# // 4+1+4+1+10+1
        //# sprintf(thead+thb_col, "%s|%s %-10s ", STYLE_DIM, STYLE_RESET_ALL, "Name");
        int tmp_col = output_my_table_sprintf(TABLE_HEAD_TYPES[2], thead+thb_col, 10, 1);
        thb_col += tmp_col;
        // 1+1+10+1
        //                          | @姓名      
        sprintf(linerow+liner_col, "+------------");
        liner_col += 13;
    }
    if(output_type[3]) { // Class
        //# // 4+1+4+1+4+1
        //# sprintf(thead+thb_col, "%s|%s %4s ", STYLE_DIM, STYLE_RESET_ALL, "Clas.");
        int tmp_col = output_my_table_sprintf(TABLE_HEAD_TYPES[3], thead+thb_col, 4, 0);
        thb_col += tmp_col;
        // 1+1+4+1
        //                          | 班级 
        sprintf(linerow+liner_col, "+------");
        liner_col += 7;
    }
    if(output_type[4]) { // Scores
        for(int i = 0; i < 5; i++)
        {
            // 4+1+4+1+4+1
            int tmp_col = output_my_table_sprintf(SCORE_TABLE[i], thead+thb_col, 5, 0);
            thb_col += tmp_col;
            // 1+1+4+1
            //                          | Sco. 
            sprintf(linerow+liner_col, "+-------");
            liner_col += 8;
        }
    }
    if(output_type[5]) { // Total
        //# // 4+1+4+1+4+1
        //# sprintf(thead+thb_col,"%s|%s %4s ", STYLE_DIM, STYLE_RESET_ALL, "Tot.");
        int tmp_col = output_my_table_sprintf(TABLE_HEAD_TYPES[5], thead+thb_col, 5, 0);
        thb_col += tmp_col;
        // 1+1+4+1
        sprintf(linerow+liner_col, "+-------");
        liner_col += 8;
    }
    sprintf(thead+thb_col, "%s|%s\n\0", STYLE_DIM, STYLE_RESET_ALL);
    sprintf(linerow+liner_col, "+%s\n\0", STYLE_RESET_ALL);
    thb_col += 10;
    liner_col += 6;

    printf(linerow); printf(thead); printf(linerow);// 输出头部

    // 表格体
    for(int i = 0; i < _len; i++)
    {
        thb_col = 0;
        char tbody[1000];
        if(output_type[0]) {sprintf(tbody+thb_col, "%s|%s %4d ", STYLE_DIM, STYLE_RESET_ALL, i+1); thb_col+=15;} // Rank
        if(output_type[1]) {sprintf(tbody+thb_col, "%s|%s %6d ", STYLE_DIM, STYLE_RESET_ALL, _stu[i].id); thb_col+=17;} // ID
        if(output_type[2]) { // Name
            int tmp_col = output_my_table_sprintf(_stu[i].name, tbody+thb_col, 10, 1);
            thb_col += tmp_col;
        }
            
        if(output_type[3]) {sprintf(tbody+thb_col, "%s|%s %4d ", STYLE_DIM, STYLE_RESET_ALL, _stu[i].cla); thb_col+=15;} // Class
        if(output_type[4]) { // Scores
            for(int j = 0; j < 5; j++)
            {
                sprintf(tbody+thb_col, "%s|%s %5.1lf ", STYLE_DIM, STYLE_RESET_ALL, _stu[i].score[j]);
                thb_col += 16;
            }
        }
        if(output_type[5]) {sprintf(tbody+thb_col, "%s|%s %5.1lf ", STYLE_DIM, STYLE_RESET_ALL, _stu[i].totscore); thb_col += 16;} // Total
        sprintf(tbody+thb_col, "%s|%s\n\0", STYLE_DIM, STYLE_RESET_ALL); thb_col += 10;
        printf(tbody); printf(linerow); // 输出一行
    }
}

//////////////////////
/// Major Functions //
//////////////////////

/**
 * @brief 学生信息录入。
 * @param _stu 一个 `Student` 结构体。
 * @return 0: 超出范围; 1: 成功。
 */
USH student_input(struct Student _stu)
{
    if(_stu.id < 0 || _stu.id >= MAXSTUID) // 出错，超出范围
    {
        printf("%s学生学号不能为负，或大于等于 %d%s\n", OUTPUT_ERR, MAXSTUID, STYLE_RESET_ALL);
        return 0;
    }
    if(_stu.cla < 0 || _stu.cla >= MAXCLASSID) // 同上
    {
        printf("%s班级编号不能为负，或大于等于 %d%s\n", OUTPUT_ERR, MAXCLASSID, STYLE_RESET_ALL);
        return 0;
    }
    if(stucnt > MAXN) // 同上
    {
        printf("%s录入学生总数已超出上限 %d%s\n", OUTPUT_ERR, MAXN, STYLE_RESET_ALL);
        return 0;
    }
    if(stuid[_stu.id] != NULL) // 学生已存在
    {
        printf("%s学生已存在%s\n", OUTPUT_ERR, STYLE_RESET_ALL);
        return 0;
    }
    stu[stucnt] = _stu; // 录入到总数据库

    if(classid[_stu.cla] == NULL) // 班级不存在，记录班级对应的 SchClass 地址
    {
        if(classcnt > MAXCLASS)
        {
            printf("%s录入班级总数已超出上限 %d%s\n", OUTPUT_ERR, MAXCLASS, STYLE_RESET_ALL);
            return 0;
        }
        stuclass[classcnt].totscore = 0.0;
        stuclass[classcnt].memcnt = 0;
        stuclass[classcnt].id = _stu.cla;
        classid[_stu.cla] = &stuclass[classcnt++];
    }
    // 记录学生
    classid[_stu.cla]->members[classid[_stu.cla]->memcnt++] = &stu[stucnt];
    classid[_stu.cla]->totscore += _stu.totscore;
    classid[_stu.cla]->average = classid[_stu.cla]->totscore*1.0 / classid[_stu.cla]->memcnt*1.0;
    stu[stucnt].clark = classid[_stu.cla]->memcnt;

    stuid[_stu.id] = &stu[stucnt]; // 记录学号对应的 Student 地址

    stucnt++; // 人数++

    return 1;
}
/**
 * @brief 按学号获取学生信息
 * @param _id 学号
 * @return 0: 不存在; 1: 成功
 */
USH student_get_by_id(int _id)
{
    if(stuid[_id] == NULL) return 0;
    struct Student out_arr[1] = {*stuid[_id]}; // 解地址
    output_table_format(0b011111, out_arr, 1);

    return 1;
}
/**
 * @brief 获取某门成绩最高的学生信息
 * @param _grade_type 成绩类型，见 SCORE_TABLE
 * @see SCORE_TABLE
 */
/*{
首先，肯定要根据 `_grade_type` 排序
  `_grade_type` 就是 `SCORE_TABLE` 里的第一个下标，代表成绩；
  也是 stu[i].score 里的下标。
写排序
然后输出。
}*/

void student_get_by_higest_grades(int _grade_type)
{
    double max_score = 0;// 目前获取到的哪一门的最大分数
    // 0 <= i < stucnt；  stu[i].score[_grade_type]
    int output_cnt = 0;
    struct Student output_stu[MAXN];
    int i;
    for(i = 0; i < stucnt; i++)
    { // 先获取最大分数
        if(_grade_type == 5 ?
            stu[i].totscore > max_score
           :stu[i].score[_grade_type] > max_score
        ) max_score=stu[i].score[_grade_type];
    }
    for(i = 0; i < MAXSTUID; i++)
    {
        if(stuid[i] == NULL) continue;
        if(_grade_type == 5?
            stuid[i]->totscore == max_score
           :stuid[i]->score[_grade_type] == max_score
        ) { // 把成绩一样高的人按学号输出
            output_stu[output_cnt++] = *stuid[i];
        }
    }
    output_table_format(0b011110, output_stu, output_cnt);
}

/**
 * @brief 按总分数升序输出
 */
void student_sort_by_grades()
{
    struct Student *output_stu = stu; // 直接改 stu 吧。。。
    int output_cnt = 0;
    for(int i = 0; i < stucnt; i++)
    {
        for(int j = 0; j < stucnt - i; j++)
        {
            if(output_stu[j+1].totscore > output_stu[j].totscore) // 后面人的总分大于前面人的总分
            {
                struct Student tmp = output_stu[j+1]; // 交换
                output_stu[j+1] = output_stu[j];
                output_stu[j] = tmp;
            }
        }
    }
    output_table_format(0b111001, output_stu, stucnt);
}

void main_csv()
{
    ;
}

/// @brief 设置正确的 locale；在 Windows 中启用虚拟终端序列以正常使用 ANSI 转义序列，并设置代码页为 65001
void main_env_setup()
{
    setlocale(LC_ALL, "zh_CN.UTF-8"); // 正确处理中文
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    SetConsoleOutputCP(CP_UTF8);// 在 Windows 中正确处理中文
    SetConsoleCP(CP_UTF8); // 同上

    // 以下是为了 conhost.exe（Windows10-or-earlier 默认控制台窗口主机，Windows11 可以使用的控制台窗口主机） 正确处理 ANSI 转义序列 `\033`。
    // 开启控制台虚拟终端序列，see: https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#output-sequences 。
    // 网页不要用 /zh-cn/ 指定语言，简中人机翻译太难绷了...
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 句柄
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode); // 获取当前模式
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
        #pragma message("The MinGW is too old!!!!\nWhy do we still use Bloodshed DEV-C++???")
    #endif
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // 开启
    WINBOOL res = SetConsoleMode(hOut, dwMode); // 应用设置
#endif
}

/// @brief 主要输入交互函数
void main_interact()
{
    while(1)
    {
        printf("%s\n键入数字进行对应操作: \n[1]输入学生信息\n[2]显示学生信息\n[3]按学号查找学生信息\n[4]显示某课程最高分的学生信息\n[5]显示总分最高的学生信息\n[6]按总分升序输出\n[7]班级对比\n[0]退出\n>> %s", STYLE_RESET_ALL, STYLE_LIGHT_CYAN);
        USH op; scanf("%hu", &op); printf("%s\n", STYLE_RESET_ALL);
        switch(op) {
            case 1:
            {
                printf("%s[1]输入学生信息%s\n", STYLE_LIGHT_BLUE, STYLE_RESET_ALL);

                struct Student input_stu;
                double totscore = 0.0;
                printf("学号>> %s", STYLE_LIGHT_CYAN); scanf("%d", &input_stu.id); printf(STYLE_RESET_ALL);
                printf("姓名>> %s", STYLE_LIGHT_CYAN); scanf("%s", &input_stu.name); printf(STYLE_RESET_ALL);
                printf("班级>> %s", STYLE_LIGHT_CYAN); scanf("%d", &input_stu.cla); printf(STYLE_RESET_ALL);
                for(int i = 0; i < 5; i++)
                {
                    printf("成绩-%s>> %s", SCORE_TABLE[i], STYLE_LIGHT_CYAN);
                    scanf("%lf", &input_stu.score[i]);
                    printf(STYLE_RESET_ALL);
                    totscore += input_stu.score[i];
                }
                input_stu.totscore = totscore;

                USH res = student_input(input_stu);
                if(res == 0) printf("\n录入出错。\n\n");
                else printf("\n录入成功完成。\n\n");

                break;
            }
            case 2: // 这个不封装，因为只是输入输出
            {
                printf("%s[2]显示学生信息%s\n", STYLE_LIGHT_BLUE, STYLE_RESET_ALL);

                USH op2;
                printf("[1]全部显示\n[2]按班级分开\n>> %s", STYLE_LIGHT_CYAN); scanf("%d", &op2);
                printf(STYLE_RESET_ALL);
                if(op2 == 1)
                {
                    printf("\n总人数: %d\n\n", stucnt);
                    output_table_format(0b011101, stu, stucnt);
                }
                else if(op2 == 2)
                {
                    printf("\n总班级数: %d\n\n", classcnt);
                    for(int i = 0; i < classcnt; i++) // 班级
                    {
                        struct Student out_arr[MAXN];
                        int out_arr_cnt = 0;
                        printf("班级: %d; 总分: %.1lf; 总分平均分: %.2lf\n", stuclass[i].id, stuclass[i].totscore, stuclass[i].average);
                        for(int j = 0; j < stuclass[i].memcnt; j++) // 班级内第 j 个人
                        {
                            struct Student out = *(stuclass[i].members[j]);
                            out_arr[out_arr_cnt++] = out;
                        }
                        output_table_format(0b011001, out_arr, out_arr_cnt);
                        printf("\n");
                    }
                }
                printf("\n");

                break;
            }
            case 3:
            {
                printf("%s[3]按学号查找学生信息%s\n", STYLE_LIGHT_BLUE, STYLE_RESET_ALL);

                int input_id;
                printf("学号?>> %s", STYLE_LIGHT_CYAN); scanf("%d", &input_id); printf("%s\n", STYLE_RESET_ALL);
                if(input_id < 0 || input_id >= MAXSTUID)
                {
                    printf("%s学号不能为负，或大于等于 %d%s\n", OUTPUT_ERR, MAXSTUID, STYLE_RESET_ALL);
                    break;
                }
                int res = student_get_by_id(input_id);
                if(!res) printf("%s不存在学号为 %d 的学生%s\n", OUTPUT_WARN, input_id, STYLE_RESET_ALL);
                printf("\n");

                break;
            }
            case 4:
            {
                printf("%s[4]显示某课程最高分的学生信息%s\n", STYLE_LIGHT_BLUE, STYLE_RESET_ALL);

                for(int i = 0; i < 5; i++) {printf("[%d]%s\n", i+1, SCORE_TABLE[i]);}
                int course_id = 0;
                printf("课程?>> %s", STYLE_LIGHT_CYAN); scanf("%d", &course_id); printf("%s\n", STYLE_RESET_ALL);
                if(course_id < 1 || course_id > 5)
                {
                    printf("%s错误的课程编号%s\n", OUTPUT_ERR, STYLE_RESET_ALL);
                    break;
                }
                student_get_by_higest_grades(course_id - 1);
                printf("\n");
                
                break;
            }
            case 5:
            {
                printf("%s[5]显示总分最高的学生信息%s\n", STYLE_LIGHT_BLUE, STYLE_RESET_ALL);

                printf("\n");
                student_get_by_higest_grades(5);
                printf("\n");

                break;
            }
            case 6:
            {
                printf("%s[6]按总分升序输出%s\n", STYLE_LIGHT_BLUE, STYLE_RESET_ALL);
                student_sort_by_grades();
                break;
            }
            // case 7: // (optional)
            // {
            //     printf("%s[7]班级对比%s\n", STYLE_LIGHT_BLUE, STYLE_RESET_ALL);
            //     break;
            // }
            case 0:
            {
                printf("%s[0]退出%s\n", STYLE_LIGHT_BLUE, STYLE_RESET_ALL);
                return;
                break;
            }
            default: printf("错误的指令。\n\n");
        }
    }
}

//////////////////////////
/// The `main` function //
//////////////////////////

/**
 * @brief 主函数。
 * @details 主函数等待用户交互。
 * @return int
 */
int main(int argc, char *argv[])
{
    main_env_setup();

    main_interact();

    return 0;
}