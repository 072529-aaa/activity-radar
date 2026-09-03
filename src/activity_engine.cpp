// activity_engine.cpp - 活动雷达核心搜索引擎 (C++)
// 编译: g++ -std=c++17 -O2 -static -o activity_engine.exe activity_engine.cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

using namespace std;

// ===== Windows 宽字符命令行转 UTF-8 =====
#ifdef _WIN32
static vector<string> getUtf8Args() {
    vector<string> args;
    int wargc = 0;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if (wargv) {
        for (int i = 0; i < wargc; i++) {
            int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                string s(len, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, &s[0], len, nullptr, nullptr);
                if (!s.empty() && s.back() == '\0') s.pop_back();
                args.push_back(s);
            }
        }
        LocalFree(wargv);
    }
    return args;
}
#endif

// ===== 活动数据结构 =====
struct Volunteer {
    bool recruiting = false;
    vector<string> roles;
    int count = 0;
    string deadline;
    vector<string> benefits;
    string contact;
};

struct Activity {
    int id;
    string type;
    string category;
    string city;
    bool isWuhan;
    string title;
    string location;
    string date;
    string endDate;
    string organizer;
    string description;
    vector<string> tags;
    Volunteer volunteer;
};

// ===== 内置活动数据 =====
vector<Activity> loadActivities() {
    return {
        {1,"ai","博览会","武汉",true,"2026武汉（国际）人工智能与机器人博览会","武汉国际博览中心","2026-09-04","2026-09-06","武汉市人民政府 / 中国人工智能学会","华中地区规模最大的AI与机器人专业展会，设置工业智能装备、商业服务机器人等七大展区，汇聚500余家企业。",{"大模型","机器人","工业落地","具身智能"},{true,{"展会引导","技术翻译","媒体接待","会场服务"},120,"2026-08-30",{"志愿者证书","免费餐饮","展会纪念品"},"volunteer@wuhan-ai-expo.com"}},
        {2,"ai","产业大会","武汉",true,"聚合智能产业发展大会2026","武汉光谷科技会展中心","2026-09-08","2026-09-09","武汉东湖新技术开发区管委会","聚焦聚合智能产业生态，开设具身机器人与产业协同发展论坛、AI与行业融合论坛。",{"聚合智能","具身机器人","产业对接","项目路演"},{true,{"签到接待","论坛会务","路演协助","嘉宾引导"},60,"2026-09-01",{"大会证书","工作餐","行业人脉"},"027-8765xxxx"}},
        {3,"ai","博览会","武汉",true,"武汉AI系列博览会（三展联动）","武汉国际博览中心","2026-09-22","2026-09-24","湖北省经信厅 / 武汉市科技局","三展联动，1200余家展商参展，配套具身智能大模型现场推理秀。",{"AI应用","算力","人形机器人","消费电子"},{true,{"展区服务","技术支持助理","观众引导","后勤保障"},200,"2026-09-15",{"志愿服务证明","三餐保障","交通补贴"},"微信公众号：武汉AI博览会"}},
        {4,"ai","产业大会","武汉",true,"ACCON 2026高端芯片产业创新发展大会","武汉·中国光谷","2026-10-22","2026-10-23","中国半导体行业协会 / 武汉市政府","聚焦AI底层算力与芯片支撑，配套AI-NATIVE元生代创业营招募。",{"芯片","算力","AI-NATIVE","创业营"},{false,{},0,"",{},""}},
        {5,"ai","创新大赛","武汉",true,"2026全国AI智能体创新开发大赛（武汉主赛场）","武汉光谷创业街","2026-09-15","2026-12-10","湖北省科技厅 / 武汉市人才工作局","设武汉为主赛场，面向35岁以下青年开发者，覆盖智能体全栈开发与场景落地两大赛道。",{"智能体","开发者大赛","创业孵化","免费算力"},{true,{"赛事运营","技术评审助理","选手服务","直播协助"},40,"2026-09-10",{"赛事证书","专家指导","创业资源"},"www.ai-agent-contest.cn"}},
        {6,"ai","创新大赛","武汉",true,"\"灵智江城·赛博龙虾\"智能体创新大赛","武汉江岸区·汉口文创谷","2026-09-20","2026-11-30","武汉市江岸区人民政府","聚焦涉江治水、智慧教育、智慧医疗等五大本土场景，设技术挑战、场景创新两大赛道。",{"智能体","本土场景","智慧医疗","创业扶持"},{true,{"活动执行","选手对接","物料管理","现场摄影"},30,"2026-09-18",{"志愿者证明","龙虾节福利","创业园区资源"},"报名微信：赛博龙虾助手"}},
        {7,"ai","厂商巡展","武汉",true,"火山引擎AI创新巡展·光谷站","武汉光谷凯悦酒店","2026-10-12","2026-10-12","字节跳动 / 火山引擎","发布ArkClaw智能体平台万人体验季，展示Trae AI编程助手、扣子Coze等产品。",{"火山引擎","Trae","Coze","智能体平台"},{false,{},0,"",{},""}},
        {8,"ai","黑客松","武汉",true,"武汉AI智能体黑客松·光谷专场","武汉光谷软件园","2026-10-18","2026-10-19","武汉东湖高新区 / 光谷创业咖啡","48小时极限开发，聚焦AI智能体场景创新，设一等奖5万元。",{"黑客松","48小时","智能体","创业"},{true,{"赛事志愿者","技术支持","餐饮保障","媒体宣传"},25,"2026-10-10",{"纪念T恤","免费餐饮","创业者人脉"},"hackathon@opticsvalley.com"}},
        {9,"marathon","半程马拉松","武汉",true,"2026武汉后官湖半程马拉松","武汉蔡甸区后官湖湿地公园","2026-10-18","2026-10-18","武汉市体育局 / 蔡甸区人民政府","环绕后官湖绿道的经典半程赛事，设半程马拉松和健康跑，参赛规模8000人。",{"半程马拉松","环湖赛道","楚文化奖牌","健康跑"},{true,{"起点服务","赛道补给","终点引导","医疗辅助","存包服务"},500,"2026-10-05",{"志愿者服装","赛事证书","工作餐","保险"},"www.houguanhu-marathon.com"}},
        {10,"marathon","半程马拉松","武汉",true,"2026武汉光谷半程马拉松","武汉光谷广场（起点）","2026-11-08","2026-11-08","武汉东湖新技术开发区管委会","穿越光谷核心区的科技主题马拉松，设半程马拉松和迷你跑，参赛规模10000人。",{"半程马拉松","科技主题","光谷地标","高校赛道"},{true,{"起点检录","赛道指引","补给站服务","完赛服务","医疗志愿者"},600,"2026-10-25",{"定制志愿者服","志愿服务证书","赛事包"},"微信公众号：光谷马拉松"}},
        {11,"marathon","迷你马拉松","武汉",true,"2026武汉江滩迷你马拉松","武汉汉口江滩","2026-09-27","2026-09-27","武汉市江岸区文化和旅游局","长江之滨的亲子友好型迷你马拉松，设10km竞速组和5km亲子组。",{"迷你马拉松","亲子跑","江滩风景","新手友好"},{true,{"签到发放","赛道服务","亲子区协助","医疗点辅助"},150,"2026-09-20",{"志愿者T恤","活动证书","免费餐饮"},"027-8276xxxx"}},
        {12,"ai","黑客松","上海",false,"外滩黑客松·AI Coding大赛","上海外滩金融中心","2026-07-18","2026-07-20","上海市黄浦区 / 多家AI厂商联合","国内头部AI Coding平台首次集体赛事，Trae、通义灵码、CodeBuddy、Comate联合承办。",{"AI Coding","黑客松","Trae","多平台联合"},{false,{},0,"",{},""}},
        {13,"ai","创新大赛","北京",false,"百度搜索·文心智能体创新大赛","北京百度大厦（决赛）","2026-08-01","2026-11-30","百度 / 英伟达","全球规模较大的智能体专项赛事，总奖金池超百万元。",{"文心一言","智能体","百度","英伟达"},{false,{},0,"",{},""}},
        {14,"ai","创新大赛","杭州",false,"通义千问AI挑战赛","杭州阿里巴巴西溪园区","2026-08-15","2026-12-15","阿里云 / 英伟达 / 天池平台","设Code Qwen算法赛道、Agent Builder创意赛道。",{"通义千问","Qwen","天池","Agent"},{false,{},0,"",{},""}},
        {15,"ai","黑客松","深圳",false,"腾讯云黑客松Agent应用创新挑战赛","深圳腾讯滨海大厦","2026-09-10","2026-09-12","腾讯云","设企业微信智能体、公众号智能体、创新创意三大赛道。",{"腾讯云","元器","企业微信","智能体"},{true,{"赛事服务","技术支持","选手接待"},35,"2026-09-05",{"腾讯云周边","赛事证书","工作餐"},"hackathon@tencent.com"}},
        {16,"ai","开发者大赛","合肥",false,"iFLYTEK AI开发者大赛","合肥科大讯飞总部","2026-07-01","2026-10-31","科大讯飞","以智能体为核心驱动赛道，覆盖办公、教育、健康、制造等垂直场景。",{"讯飞星火","星辰Agent","开发者大赛","垂直场景"},{false,{},0,"",{},""}},
        {17,"ai","高校赛事","武汉",true,"\"星火杯\"大模型应用创新赛·华中赛区","华中科技大学（武汉）","2026-10-25","2026-10-26","科大讯飞 / 华中科技大学","面向全球高校开发者，2026年奖金池25万元，华中赛区落地武汉。",{"高校赛事","星火杯","华中赛区","智能体"},{true,{"赛场服务","选手接待","技术支持","评审协助"},45,"2026-10-18",{"赛事证书","讯飞周边","工作餐","校招绿色通道"},"spark-cup@iflytek.com"}},
        {18,"marathon","全程马拉松","北京",false,"2026北京马拉松","北京天安门广场（起点）","2026-11-02","2026-11-02","中国田径协会 / 北京市体育局","国内最高水平的马拉松赛事之一，参赛规模30000人。",{"全程马拉松","金标赛事","天安门","3万人规模"},{true,{"起点服务","赛道补给","终点引导","医疗辅助","翻译服务"},3000,"2026-10-15",{"志愿者服装","赛事证书","工作餐","保险"},"www.beijing-marathon.com"}},
        {19,"marathon","全程马拉松","上海",false,"2026上海马拉松","上海外滩金牛广场（起点）","2026-11-23","2026-11-23","中国田径协会 / 上海市体育局","国际田联白金标赛事，参赛规模38000人。",{"全程马拉松","白金标","外滩","3.8万人"},{true,{"起点检录","赛道服务","补给站","完赛服务","医疗志愿者"},4000,"2026-10-31",{"上马志愿者服","志愿服务证书","餐饮保障","保险"},"www.shmarathon.com"}},
        {20,"marathon","全程马拉松","杭州",false,"2026杭州马拉松","杭州黄龙体育中心（起点）","2026-11-16","2026-11-16","中国田径协会 / 杭州市体育局","国际田联金标赛事，设全马、半马、迷你跑，参赛规模35000人。",{"全程马拉松","金标赛事","西湖","钱塘江"},{true,{"赛事服务","赛道指引","补给站","医疗辅助"},2500,"2026-10-25",{"杭马志愿者服","证书","工作餐","保险"},"www.hzim.org"}},
        {21,"marathon","全程马拉松","成都",false,"2026成都马拉松","成都金沙遗址博物馆（起点）","2026-10-26","2026-10-26","中国田径协会 / 成都市人民政府","国际田联金标赛事，参赛规模30000人。",{"全程马拉松","金标赛事","天府之国","美食赛道"},{true,{"起点服务","赛道补给","终点服务","医疗志愿者"},2800,"2026-10-10",{"成马志愿者服","证书","餐饮","保险"},"www.chengdu-marathon.org"}},
        {22,"marathon","全程马拉松","广州",false,"2026广州马拉松","广州天河体育中心（起点）","2026-12-07","2026-12-07","中国田径协会 / 广州市人民政府","国际田联金标赛事，参赛规模30000人。",{"全程马拉松","金标赛事","广州塔","珠江"},{true,{"赛事服务","赛道指引","补给站","医疗辅助","终点服务"},2600,"2026-11-15",{"广马志愿者服","证书","工作餐","保险"},"www.guangzhou-marathon.com"}},
        {23,"marathon","半程马拉松","南京",false,"2026南京马拉松","南京奥体中心（起点）","2026-11-09","2026-11-09","中国田径协会 / 南京市体育局","国际田联铜标赛事，设全马、半马、迷你跑，参赛规模28000人。",{"全程马拉松","半程马拉松","中山陵","玄武湖"},{true,{"起点服务","赛道补给","终点引导","医疗辅助"},2000,"2026-10-20",{"南马志愿者服","证书","餐饮","保险"},"www.nj-marathon.com"}},
        {24,"marathon","全程马拉松","长沙",false,"2026长沙马拉松","长沙贺龙体育中心（起点）","2026-10-19","2026-10-19","中国田径协会 / 长沙市体育局","国际田联铜标赛事，以\"红色马拉松\"为特色，参赛规模24000人。",{"全程马拉松","橘子洲","岳麓山","红色主题"},{true,{"赛事服务","赛道指引","补给站","医疗志愿者"},1800,"2026-10-05",{"长马志愿者服","证书","工作餐","保险"},"www.changsha-marathon.com"}},
        {25,"marathon","半程马拉松","重庆",false,"2026重庆女子半程马拉松","重庆南滨路（起点）","2026-11-02","2026-11-02","中国田径协会 / 重庆市体育局","国内唯一的国际田联铜标女子半程马拉松，完赛奖牌为精美项链设计。",{"女子半程","半程马拉松","南滨路","项链奖牌"},{true,{"起点服务","赛道补给","终点服务","医疗辅助","礼仪服务"},800,"2026-10-20",{"志愿者服装","证书","工作餐","保险"},"www.cqwomensmarathon.com"}},
        {26,"ai","技术峰会","武汉",true,"2026武汉AI+教育融合发展峰会","武汉国际会议中心","2026-10-15","2026-10-16","湖北省教育厅 / 武汉市教育局","聚焦AI技术在教育场景的落地应用，设置教育智能体产品体验区。",{"AI+教育","智能体教学","AI助教","教育科技"},{true,{"嘉宾接待","会场服务","技术支持","资料分发"},50,"2026-10-08",{"峰会证书","工作餐","教育行业人脉"},"ai-edu@wuhan.gov.cn"}},
        {27,"ai","厂商活动","武汉",true,"通义灵码开发者日·武汉站","武汉光谷创业咖啡","2026-09-28","2026-09-28","阿里云 / 通义灵码","AI编程工具深度体验活动，包含功能演示、实战Workshop、圆桌讨论。",{"通义灵码","AI编程","开发者日","Workshop"},{false,{},0,"",{},""}},
        {28,"ai","产业论坛","武汉",true,"2026中国（武汉）智能体产业生态论坛","武汉会议中心","2026-11-06","2026-11-07","中国人工智能学会 / 武汉市科技局","聚焦智能体产业生态构建，发布武汉智能体产业发展白皮书。",{"智能体产业","生态论坛","白皮书","投资对接"},{true,{"论坛会务","嘉宾接待","记录整理","现场协调"},40,"2026-10-30",{"论坛证书","工作餐","行业资源对接"},"微信公众号：武汉智能体论坛"}},
        {29,"marathon","越野跑","武汉",true,"2026武汉木兰山越野挑战赛","武汉黄陂区木兰山风景区","2026-11-15","2026-11-15","武汉市黄陂区文化和旅游局","设50km、25km、10km三个组别，累计爬升1800米，参赛规模1500人。",{"越野跑","木兰山","50km","森林赛道"},{true,{"打卡点服务","补给站","赛道指引","救援辅助"},120,"2026-11-01",{"越野赛志愿者服","证书","餐饮","保险"},"www.mulan-trail.com"}},
        {30,"ai","技术沙龙","武汉",true,"AIGC商业化路径探索沙龙·武汉站","武汉汉口江滩·共享空间","2026-09-14","2026-09-14","武汉创业者协会 / 光谷AI俱乐部","聚焦AIGC商业化落地，设置项目路演与投资人面对面环节。",{"AIGC","商业化","创业沙龙","项目路演"},{true,{"签到接待","现场执行","摄影记录","嘉宾对接"},15,"2026-09-10",{"免费参会","创业者人脉","活动证书"},"微信：光谷AI俱乐部"}}
    };
}

// ===== 工具函数 =====
string toLower(const string& s) {
    string r = s;
    for (auto& c : r) c = tolower((unsigned char)c);
    return r;
}

string getTodayStr() {
    time_t now = time(nullptr);
    struct tm t;
#if defined(_WIN32)
    localtime_s(&t, &now);
#else
    t = *localtime(&now);
#endif
    char buf[16];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    return string(buf);
}

int daysUntil(const string& dateStr) {
    int y1, m1, d1;
    sscanf(dateStr.c_str(), "%d-%d-%d", &y1, &m1, &d1);
    string today = getTodayStr();
    int y2, m2, d2;
    sscanf(today.c_str(), "%d-%d-%d", &y2, &m2, &d2);
    return (y1 - y2) * 365 + (m1 - m2) * 30 + (d1 - d2);
}

string formatDate(const string& ds) {
    int y, m, d;
    sscanf(ds.c_str(), "%d-%d-%d", &y, &m, &d);
    return to_string(m) + "月" + to_string(d) + "日";
}

string formatDateRange(const string& s, const string& e) {
    return s == e ? formatDate(s) : formatDate(s) + " - " + formatDate(e);
}

string typeLabel(const string& t) {
    return t == "ai" ? "AI活动" : t == "marathon" ? "马拉松" : "其他";
}

// ===== JSON 输出 =====
string escapeJson(const string& s) {
    string r;
    for (char c : s) {
        switch (c) {
            case '"': r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\n': r += "\\n"; break;
            case '\r': r += "\\r"; break;
            case '\t': r += "\\t"; break;
            default: r += c;
        }
    }
    return r;
}

void outputJson(const vector<Activity>& list) {
    cout << "[";
    for (size_t i = 0; i < list.size(); i++) {
        const auto& a = list[i];
        if (i > 0) cout << ",";
        cout << "{\"id\":" << a.id
             << ",\"type\":\"" << a.type << "\""
             << ",\"category\":\"" << escapeJson(a.category) << "\""
             << ",\"city\":\"" << escapeJson(a.city) << "\""
             << ",\"isWuhan\":" << (a.isWuhan ? "true" : "false")
             << ",\"title\":\"" << escapeJson(a.title) << "\""
             << ",\"location\":\"" << escapeJson(a.location) << "\""
             << ",\"date\":\"" << a.date << "\""
             << ",\"endDate\":\"" << a.endDate << "\""
             << ",\"organizer\":\"" << escapeJson(a.organizer) << "\""
             << ",\"description\":\"" << escapeJson(a.description) << "\""
             << ",\"tags":[";
        for (size_t j = 0; j < a.tags.size(); j++) {
            if (j > 0) cout << ",";
            cout << "\"" << escapeJson(a.tags[j]) << "\"";
        }
        cout << "],\"volunteer\":{\"recruiting\":" << (a.volunteer.recruiting ? "true" : "false")
             << ",\"count\":" << a.volunteer.count
             << ",\"deadline\":\"" << a.volunteer.deadline << "\""
             << ",\"contact\":\"" << escapeJson(a.volunteer.contact) << "\""
             << ",\"roles":[";
        for (size_t j = 0; j < a.volunteer.roles.size(); j++) {
            if (j > 0) cout << ",";
            cout << "\"" << escapeJson(a.volunteer.roles[j]) << "\"";
        }
        cout << "],\"benefits\":[";
        for (size_t j = 0; j < a.volunteer.benefits.size(); j++) {
            if (j > 0) cout << ",";
            cout << "\"" << escapeJson(a.volunteer.benefits[j]) << "\"";
        }
        cout << "]}}";
    }
    cout << "]" << endl;
}

// ===== 文本表格输出 =====
void outputText(const vector<Activity>& list) {
    if (list.empty()) {
        cout << "未找到匹配的活动。" << endl;
        return;
    }
    cout << "\n=== 活动雷达 · 搜索结果 (" << list.size() << " 场) ===\n" << endl;
    for (size_t i = 0; i < list.size(); i++) {
        const auto& a = list[i];
        int days = daysUntil(a.date);
        string dateInfo = days < 0 ? "已结束" : days == 0 ? "今天" : days == 1 ? "明天" : to_string(days) + "天后";
        cout << "[" << (i+1) << "] " << a.title;
        if (a.isWuhan) cout << " [武汉]";
        cout << endl;
        cout << "    类型: " << typeLabel(a.type) << " · " << a.category << endl;
        cout << "    时间: " << formatDateRange(a.date, a.endDate) << " (" << dateInfo << ")" << endl;
        cout << "    地点: " << a.city << " · " << a.location << endl;
        cout << "    主办: " << a.organizer << endl;
        if (a.volunteer.recruiting) {
            cout << "    志愿者: 招募中 (" << a.volunteer.count << "人, 截止" << a.volunteer.deadline << ")" << endl;
        }
        cout << endl;
    }
}

// ===== 主函数 =====
int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    vector<string> args = getUtf8Args();
#else
    vector<string> args;
    for (int i = 0; i < argc; i++) args.push_back(argv[i]);
#endif
    int nargs = (int)args.size();

    string city = "武汉";
    string type = "all";
    string search = "";
    string sortBy = "date";
    bool volunteerOnly = false;
    bool jsonOutput = false;
    bool listCities = false;

    for (int i = 1; i < nargs; i++) {
        string arg = args[i];
        if (arg == "--city" && i+1 < nargs) city = args[++i];
        else if (arg == "--type" && i+1 < nargs) type = args[++i];
        else if (arg == "--search" && i+1 < nargs) search = args[++i];
        else if (arg == "--sort" && i+1 < nargs) sortBy = args[++i];
        else if (arg == "--volunteer") volunteerOnly = true;
        else if (arg == "--json") jsonOutput = true;
        else if (arg == "--cities") listCities = true;
        else if (arg == "--today") { cout << getTodayStr() << endl; return 0; }
        else if (arg == "--help" || arg == "-h") {
            cout << "活动雷达 C++ 搜索引擎\n"
                 << "用法: activity_engine [选项]\n\n"
                 << "选项:\n"
                 << "  --city <城市>     筛选城市 (默认: 武汉, 用 '全部' 显示所有)\n"
                 << "  --type <类型>     ai / marathon / all (默认: all)\n"
                 << "  --search <关键词> 搜索关键词\n"
                 << "  --sort <方式>     date / wuhan / volunteer (默认: date)\n"
                 << "  --volunteer       仅显示招募志愿者的活动\n"
                 << "  --json            输出JSON格式 (供Python调用)\n"
                 << "  --cities          列出所有可用城市\n"
                 << "  --help            显示帮助\n";
            return 0;
        }
    }

    vector<Activity> all = loadActivities();

    if (listCities) {
        vector<string> cities;
        for (const auto& a : all) {
            if (find(cities.begin(), cities.end(), a.city) == cities.end())
                cities.push_back(a.city);
        }
        if (jsonOutput) {
            cout << "[";
            for (size_t i = 0; i < cities.size(); i++) {
                if (i > 0) cout << ",";
                cout << "\"" << escapeJson(cities[i]) << "\"";
            }
            cout << "]" << endl;
        } else {
            cout << "可用城市: ";
            for (size_t i = 0; i < cities.size(); i++) {
                if (i > 0) cout << ", ";
                cout << cities[i];
            }
            cout << endl;
        }
        return 0;
    }

    string today = getTodayStr();
    vector<Activity> filtered;
    for (const auto& a : all) {
        if (city != "全部" && a.city != city) continue;
        if (type == "ai" && a.type != "ai") continue;
        if (type == "marathon" && a.type != "marathon") continue;
        if (volunteerOnly && !a.volunteer.recruiting) continue;
        if (!search.empty()) {
            string q = toLower(search);
            string haystack = toLower(a.title + a.location + a.organizer + a.description);
            for (const auto& t : a.tags) haystack += " " + toLower(t);
            if (haystack.find(q) == string::npos) continue;
        }
        if (a.date < today) continue;
        filtered.push_back(a);
    }

    if (sortBy == "wuhan") {
        sort(filtered.begin(), filtered.end(), [](const Activity& a, const Activity& b) {
            if (a.isWuhan != b.isWuhan) return a.isWuhan;
            return a.date < b.date;
        });
    } else if (sortBy == "volunteer") {
        sort(filtered.begin(), filtered.end(), [](const Activity& a, const Activity& b) {
            if (a.volunteer.recruiting != b.volunteer.recruiting) return a.volunteer.recruiting;
            return a.date < b.date;
        });
    } else {
        sort(filtered.begin(), filtered.end(), [](const Activity& a, const Activity& b) {
            return a.date < b.date;
        });
    }

    if (jsonOutput) outputJson(filtered);
    else outputText(filtered);

    return 0;
}
