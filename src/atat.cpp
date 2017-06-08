#include<algorithm>
#include"atat.h"
#include<cctype>
#include<cstdlib>

namespace atat
{
    Command::Command(const shared_ptr<Row>&row_):row_(row_) {}

    Row*Command::row() {return row_.get();}

    void context::setup()
    {
        CloseHandle=::CloseHandle;
        CreateEvent=::CreateEvent;
        FindWindowW=::FindWindowW;
        GetDoubleClickTime=::GetDoubleClickTime;
        GetForegroundWindow=::GetForegroundWindow;
        GetLastError=::GetLastError;
        GetSystemMetrics=::GetSystemMetrics;
        GetWindowRect=::GetWindowRect;
        SendInput=::SendInput;
        SetConsoleCtrlHandler=::SetConsoleCtrlHandler;
        SetEvent=::SetEvent;
        WaitForSingleObject=::WaitForSingleObject;

        err=&cerr;
        in=&cin;
        out=&cout;
    }

    Event::Event()
    {
        handle_=ct().CreateEvent(NULL,TRUE,FALSE,NULL);
        if(handle_==NULL)
            throw runtime_error(describe
            ("function:'CreateEvent':failed(",ct().GetLastError(),")"));
    }

    Event::~Event() {ct().CloseHandle(handle_);}

    const HANDLE&Event::handle() {return handle_;}

    void Event::set()
    {
        if(ct().SetEvent(handle_)==FALSE)
            throw runtime_error(describe
            ("function:'SetEvent':failed(",ct().GetLastError(),")"));
    }

    KeyCommand::KeyCommand(const shared_ptr<Row>&row_):Command(row_)
    {
        if(row_->tokens().size()!=3)
            throw runtime_error(describe
            ("row:'",row_->description(),"':wrong number of tokens"));
        string key=row_->tokens().at(2);
        string keyAsLC=to_lower_case(key);
        static const map<string,WORD> keyMap(
        {
            {to_lower_case("ESCAPE"),      0x01},
            {to_lower_case("1"),           0x02},
            {to_lower_case("2"),           0x03},
            {to_lower_case("3"),           0x04},
            {to_lower_case("4"),           0x05},
            {to_lower_case("5"),           0x06},
            {to_lower_case("6"),           0x07},
            {to_lower_case("7"),           0x08},
            {to_lower_case("8"),           0x09},
            {to_lower_case("9"),           0x0A},
            {to_lower_case("0"),           0x0B},
            {to_lower_case("MINUS"),       0x0C},
            {to_lower_case("EQUALS"),      0x0D},
            {to_lower_case("BACK"),        0x0E},
            {to_lower_case("TAB"),         0x0F},
            {to_lower_case("Q"),           0x10},
            {to_lower_case("W"),           0x11},
            {to_lower_case("E"),           0x12},
            {to_lower_case("R"),           0x13},
            {to_lower_case("T"),           0x14},
            {to_lower_case("Y"),           0x15},
            {to_lower_case("U"),           0x16},
            {to_lower_case("I"),           0x17},
            {to_lower_case("O"),           0x18},
            {to_lower_case("P"),           0x19},
            {to_lower_case("LBRACKET"),    0x1A},
            {to_lower_case("RBRACKET"),    0x1B},
            {to_lower_case("RETURN"),      0x1C},
            {to_lower_case("LContol"),     0x1D},
            {to_lower_case("A"),           0x1E},
            {to_lower_case("S"),           0x1F},
            {to_lower_case("D"),           0x20},
            {to_lower_case("F"),           0x21},
            {to_lower_case("G"),           0x22},
            {to_lower_case("H"),           0x23},
            {to_lower_case("J"),           0x24},
            {to_lower_case("K"),           0x25},
            {to_lower_case("L"),           0x26},
            {to_lower_case("SEMICOLON"),   0x27},
            {to_lower_case("APOSTROPHE"),  0x28},
            {to_lower_case("GRAVE"),       0x29},
            {to_lower_case("LSHIFT"),      0x2A},
            {to_lower_case("BACKSLASH"),   0x2B},
            {to_lower_case("Z"),           0x2C},
            {to_lower_case("X"),           0x2D},
            {to_lower_case("C"),           0x2E},
            {to_lower_case("V"),           0x2F},
            {to_lower_case("B"),           0x30},
            {to_lower_case("N"),           0x31},
            {to_lower_case("M"),           0x32},
            {to_lower_case("COMMA"),       0x33},
            {to_lower_case("PERIOD"),      0x34},
            {to_lower_case("SLASH"),       0x35},
            {to_lower_case("RSHIFT"),      0x36},
            {to_lower_case("MULTIPLY"),    0x37},
            {to_lower_case("LMENU"),       0x38},
            {to_lower_case("SPACE"),       0x39},
            {to_lower_case("CAPITAL"),     0x3A},
            {to_lower_case("F1"),          0x3B},
            {to_lower_case("F2"),          0x3C},
            {to_lower_case("F3"),          0x3D},
            {to_lower_case("F4"),          0x3E},
            {to_lower_case("F5"),          0x3F},
            {to_lower_case("F6"),          0x40},
            {to_lower_case("F7"),          0x41},
            {to_lower_case("F8"),          0x42},
            {to_lower_case("F9"),          0x43},
            {to_lower_case("F10"),         0x44},
            {to_lower_case("NUMLOCK"),     0x45},
            {to_lower_case("SCROLL"),      0x46},
            {to_lower_case("NUMPAD7"),     0x47},
            {to_lower_case("NUMPAD8"),     0x48},
            {to_lower_case("NUMPAD9"),     0x49},
            {to_lower_case("SUBTRACT"),    0x4A},
            {to_lower_case("NUMPAD4"),     0x4B},
            {to_lower_case("NUMPAD5"),     0x4C},
            {to_lower_case("NUMPAD6"),     0x4D},
            {to_lower_case("ADD"),         0x4E},
            {to_lower_case("NUMPAD1"),     0x4F},
            {to_lower_case("NUMPAD2"),     0x50},
            {to_lower_case("NUMPAD3"),     0x51},
            {to_lower_case("NUMPAD0"),     0x52},
            {to_lower_case("DECIMAL"),     0x53},
            {to_lower_case("F11"),         0x57},
            {to_lower_case("F12"),         0x58},
            {to_lower_case("F13"),         0x64},
            {to_lower_case("F14"),         0x65},
            {to_lower_case("F15"),         0x66},
            {to_lower_case("KANA"),        0x70},
            {to_lower_case("CONVERT"),     0x79},
            {to_lower_case("NOCONVERT"),   0x7B},
            {to_lower_case("YEN"),         0x7D},
            {to_lower_case("NUMPADEQUALS"),0x8D},
            {to_lower_case("CIRCUMFLEX"),  0x90},
            {to_lower_case("AT"),          0x91},
            {to_lower_case("COLON"),       0x92},
            {to_lower_case("UNDERLINE"),   0x93},
            {to_lower_case("KANJI"),       0x94},
            {to_lower_case("STOP"),        0x95},
            {to_lower_case("AX"),          0x96},
            {to_lower_case("UNLABELED"),   0x97},
            {to_lower_case("NUMPADENTER"), 0x9C},
            {to_lower_case("RCONTROL"),    0x9D},
            {to_lower_case("NUMPADCOMMA"), 0xB3},
            {to_lower_case("DIVIDE"),      0xB5},
            {to_lower_case("SYSRQ"),       0xB7},
            {to_lower_case("RMENU"),       0xB8},
            {to_lower_case("PAUSE"),       0xC5},
            {to_lower_case("HOME"),        0xC7},
            {to_lower_case("UP"),          0xC8},
            {to_lower_case("PRIOR"),       0xC9},
            {to_lower_case("LEFT"),        0xCB},
            {to_lower_case("RIGHT"),       0xCD},
            {to_lower_case("END"),         0xCF},
            {to_lower_case("DOWN"),        0xD0},
            {to_lower_case("NEXT"),        0xD1},
            {to_lower_case("INSERT"),      0xD2},
            {to_lower_case("DELETE"),      0xD3},
            {to_lower_case("LWIN"),        0xDB},
            {to_lower_case("RWIN"),        0xDC},
            {to_lower_case("APPS"),        0xDD},
            {to_lower_case("POWER"),       0xDE},
            {to_lower_case("SLEEP"),       0xDF},
        });
        if(keyMap.find(keyAsLC)==keyMap.end())
            throw runtime_error(describe("key:'",key,"':unknown"));
        code_=keyMap.at(keyAsLC);
    }

    void KeyCommand::send(const WORD&code,const DWORD&up)
    {
        INPUT input;
        input.type=INPUT_KEYBOARD;
        input.ki.wScan=code;
        input.ki.dwFlags=KEYEVENTF_SCANCODE|up;
        input.ki.time=0;
        input.ki.dwExtraInfo=0;
        if(ct().SendInput(1,&input,sizeof(INPUT))!=1)
            throw runtime_error(describe
            ("function:'SendInput':failed(",ct().GetLastError(),")"));
    }

    KeyDownCommand::KeyDownCommand(const shared_ptr<Row>&row_):
        KeyCommand(row_) {}

    void KeyDownCommand::execute()
    {
        send(code_,0);
        wait(ct().GetDoubleClickTime()/2);
        ct().index++;
    }

    KeyPressCommand::KeyPressCommand(const shared_ptr<Row>&row_):
        KeyCommand(row_) {}

    void KeyPressCommand::execute()
    {
        send(code_,0);
        wait(ct().GetDoubleClickTime()/2);
        send(code_,KEYEVENTF_KEYUP);
        wait(ct().GetDoubleClickTime()/2);
        ct().index++;
    }

    KeyUpCommand::KeyUpCommand(const shared_ptr<Row>&row_):
        KeyCommand(row_) {}

    void KeyUpCommand::execute()
    {
        send(code_,KEYEVENTF_KEYUP);
        wait(ct().GetDoubleClickTime()/2);
        ct().index++;
    }

    LoopBeginCommand::LoopBeginCommand(const shared_ptr<Row>&row_):
        Command(row_)
    {
        if(row_->tokens().size()>3)
            throw runtime_error(describe
            ("row:'",row_->description(),"':wrong number of tokens"));
        if(row_->tokens().size()==3)
            number_=to_number(row_->tokens().at(2));
        else number_=0;
    }

    void LoopBeginCommand::execute()
    {
        ct().index++;
        frame_begin(number_);
    }

    LoopEndCommand::LoopEndCommand(const shared_ptr<Row>&row_):Command(row_)
    {
        if(row_->tokens().size()!=2)
            throw runtime_error(describe
            ("row:'",row_->description(),"':wrong number of tokens"));
    }

    void LoopEndCommand::execute()
    {
        if(ct().frames.size()==1)
            throw runtime_error(describe
            ("loop end:no corresponding begin"));
        if(!frame_end()) ct().index++;
    }

    MouseButtonClickCommand::MouseButtonClickCommand
    (const shared_ptr<Row>&row_):MouseButtonCommand(row_) {}

    void MouseButtonClickCommand::execute()
    {
        send(0,0,0,button_);
        wait(ct().GetDoubleClickTime()/2);
        send(0,0,0,button_<<1);
        wait(ct().GetDoubleClickTime()/2);
        ct().index++;
    }

    MouseButtonCommand::MouseButtonCommand(const shared_ptr<Row>&row_):
        MouseCommand(row_)
    {
        if(row_->tokens().size()!=3)
            throw runtime_error(describe
            ("row:'",row_->description(),"':wrong number of tokens"));
        static const map<string,DWORD> buttonMap(
        {
            {"left",  MOUSEEVENTF_LEFTDOWN},
            {"middle",MOUSEEVENTF_MIDDLEDOWN},
            {"right", MOUSEEVENTF_RIGHTDOWN},
        });
        button_=buttonMap.at(row_->tokens().at(1));
    }

    MouseButtonDoubleClickCommand::MouseButtonDoubleClickCommand
    (const shared_ptr<Row>&row_):MouseButtonCommand(row_) {}

    void MouseButtonDoubleClickCommand::execute()
    {
        for(size_t i=0;i<2;i++)
        {
            send(0,0,0,button_);
            wait(ct().GetDoubleClickTime()/4);
            send(0,0,0,button_<<1);
            wait(ct().GetDoubleClickTime()/4);
        }
        ct().index++;
    }

    MouseButtonDownCommand::MouseButtonDownCommand
    (const shared_ptr<Row>&row_):MouseButtonCommand(row_) {}

    void MouseButtonDownCommand::execute()
    {
        send(0,0,0,button_);
        wait(ct().GetDoubleClickTime()/2);
        ct().index++;
    }

    MouseButtonUpCommand::MouseButtonUpCommand(const shared_ptr<Row>&row_):
        MouseButtonCommand(row_) {}

    void MouseButtonUpCommand::execute()
    {
        send(0,0,0,button_<<1);
        wait(ct().GetDoubleClickTime()/2);
        ct().index++;
    }

    void MouseCommand::send
    (
        const LONG&x,
        const LONG&y,
        const DWORD&amount,
        const DWORD&buttonAndAction
    )
    {
        INPUT input;
        input.type=INPUT_MOUSE;
        input.mi.dx=x;
        input.mi.dy=y;
        input.mi.mouseData=amount;
        input.mi.dwFlags=buttonAndAction;
        input.mi.time=0;
        input.mi.dwExtraInfo=0;
        if(ct().SendInput(1,&input,sizeof(INPUT))!=1)
            throw runtime_error(describe
            ("function:'SendInput':failed(",ct().GetLastError(),")"));
    }

    MouseMoveCommand::MouseMoveCommand(const shared_ptr<Row>&row_):
        MouseCommand(row_)
    {
        if(row_->tokens().size()!=4)
            throw runtime_error(describe
            ("row:'",row_->description(),"':wrong number of tokens"));
        x_=to_number(row_->tokens().at(2));
        y_=to_number(row_->tokens().at(3));
    }

    void MouseMoveCommand::execute()
    {
        RECT targetRect;
        targetRect.left=targetRect.top=0;
        if(ct().properties.find("target")!=ct().properties.end())
        {
            if(ct().GetWindowRect(find_target(),&targetRect)==FALSE)
                throw runtime_error(describe
                (
                    "function:'GetWindowRect':"
                    "failed(",ct().GetLastError(),")"
                ));
        }
        send
        (
            ((x_+targetRect.left)<<16)/system_metrics(SM_CXSCREEN),
            ((y_+targetRect.top)<<16)/system_metrics(SM_CYSCREEN),
            0,
            MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE
        );
        ct().index++;
    }

    MouseWheelCommand::MouseWheelCommand(const shared_ptr<Row>&row_):
        MouseCommand(row_)
    {
        if(row_->tokens().size()!=3)
            throw runtime_error(describe
            ("row:'",row_->description(),"':wrong number of tokens"));
        amount_=to_number(row_->tokens().at(2));
    }

    void MouseWheelCommand::execute()
    {
        send(0,0,(DWORD)amount_,MOUSEEVENTF_WHEEL);
        ct().index++;
    }

    NullCommand::NullCommand(const shared_ptr<Row>&row_):Command(row_){}

    void NullCommand::execute() {ct().index++;}

    Row::Row(const string&description_):description_(description_)
    {
        string::size_type pndpos=description_.find_first_of('#');
        if(pndpos==string::npos) pndpos=description_.length();
        tokens_=tokenize(description_.substr(0,pndpos)," \t");
    }

    const string&Row::description() {return description_;}

    const vector<string>&Row::tokens() {return tokens_;}

    SleepCommand::SleepCommand(const shared_ptr<Row>&row_):Command(row_)
    {
        if(row_->tokens().size()!=2)
            throw runtime_error(describe
            ("row:'",row_->description(),"':wrong number of tokens"));
        time_=to_number(row_->tokens().at(1));
    }

    void SleepCommand::execute()
    {
        wait(time_);
        ct().index++;
    }

    string chomp_cr(const string&str)
    {
        string result=str;
        if(!str.empty()&&str.back()=='\r')
            result=str.substr(0,str.length()-1);
        return result;
    }

    BOOL control_key_pressed(DWORD type)
    {
        BOOL handled=FALSE;
        if(type==CTRL_C_EVENT)
        {
            try
            {
                ct().canceled_event->set();
                handled=TRUE;
            } catch (const runtime_error&error)
            {
                (*ct().err)<<describe
                ("error(",ct().index,"):",error.what())<<endl;
            }
        }
        return handled;
    }

    context&ct()
    {
        static context context_;
        return context_;
    }

    int execute(int argc,char**argv)
    {
        int result=0;
        try
        {
            ct().properties=parse_properties(argc,argv);
            if(ct().properties.find("help")!=ct().properties.end())
            {
                (*ct().out)<<describe
                (
"usage:",argv[0]," [property[=VALUE]...]\n"
"\n"
"WHAT IS ATAT?\n"
"    ATAT is an interpreter for automatically manipulating\n"
"    keyboard and mouse on Windows.\n"
"    ATAT reads the script from standard input.\n"
"    The script is described as a list of commands.\n"
"    Basically, one operation is executed with one command.\n"
"    The list of commands are executed in order from the top,\n"
"    and when it runs to the bottom, it ends.\n"
"    Press Ctrl-C to exit while running.\n"
"\n"
"PROPERTIES\n"
"    Properties are specified by command row arguments.\n"
"    Specify a name and if it has a value, connect with an equals.\n"
"    In the following list, a name is indicated in lowercase,\n"
"    and a value is indicated in uppercase.\n"
"    \n"
"    help\n"
"        Show this document, and finish without doing anything.\n"
"    ready=TIME\n"
"        Wait for ready before running.\n"
"        TIME is time to wait in milliseconds.\n"
"    repeat[=NUMBER]\n"
"        Repeat the script.\n"
"        NUMBER is number of repetition.\n"
"        If it's not specified, it's infinite.\n"
"    silent\n"
"        Don't show the commands to be executed.\n"
"    target=CAPTION\n"
"        CAPTION is the caption of the target window.\n"
"        Execute the command when this window is active.\n"
"        Also, specify the relative coordinates of this window\n"
"        in 'mouse move' command.\n"
"        If it's not specified, execute the command any window active.\n"
"\n"
"COMMANDS\n"
"    Command consists of switches followed by parameters.\n"
"    Separete switches and parameters with spaces or tabs.\n"
"    In the following list, switches are indicated in lowercase,\n"
"    and parameters are indicated in uppercase.\n"
"    Also, if there're multiple choices on the switch,\n"
"    separate them with vertical bars.\n"
"    \n"
"    key down|up|press KEY\n"
"        Down or up or both the key on the keyboard.\n"
"        KEY is the key to be operated.\n"
"        Specify a string following 'DIK_' defined in DirectInput.\n"
"    mouse move X Y\n"
"        Move the mouse.\n"
"        X and Y are the coordinates to move the mouse.\n"
"        If target is specified, assumed to be its relative coordinates.\n"
"    mouse wheel AMOUNT\n"
"        Wheel the mouse.\n"
"        AMOUNT is the amount to wheel the mouse.\n"
"    mouse left|right|middle down|up|click|doubleclick\n"
"        Down or up or click or double click\n"
"        the left or right or middle button of the mouse.\n"
"    sleep TIME\n"
"        Do nothing for a while.\n"
"        TIME is time to wait in milliseconds.\n"
"    loop begin [NUMBER]\n"
"        Begin the loop block.\n"
"        NUMBER is number of loops. If it's not specified, it's infinite.\n"
"    loop end\n"
"        End the loop block.\n"
"\n"
"EXAMPLE\n"
"    How to write?\n"
"        At the command prompt, type:$ atat target=GAME\n"
"        Since ATAT is waiting for input, it does as follows.\n"
"            sleep 3000\n"
"            mouse move 100 200\n"
"            mouse left click\n"
"            loop begin 5\n"
"                key press A\n"
"                key press B\n"
"            loop end\n"
"    How it works?\n"
"        First, wait 3 seconds,\n"
"        and then move the mouse pointer to (100,200) in the GAME window.\n"
"        Next, click the left mouse button.\n"
"        Then enter the loop block, press the A key, then press the B key.\n"
"        Return to the beginning of the loop block,\n"
"        and repeat the A and B five times.\n"
"        If the GAME window becomes inactive halfway,\n"
"        execution will be paused.\n"
"        When it becomes active again, execution resumes.\n"
"\n"
"REMARKS\n"
"    Number format\n"
"        Follow the literal syntax for number in C.\n"
"        So, it's octal if it starts with '0',\n"
"        or hexadecimal if it starts with '0x', otherwise decimal.\n"
"    Comment\n"
"        Script can contains comments, for example:\n"
"            # Script01 to earn experience.\n"
"            \n"
"            # Wait for ready.\n"
"            sleep 3000\n"
"            \n"
"            # Repeat infinitely.\n"
"            loop begin\n"
"                key press TAB # Target an enemy.\n"
"                key press 1   # Attack the target.\n"
"            loop end\n"
"        The string following the '#' is treated as a comment.\n"
"        Also, empty rows just skip.\n"
                );
            } else
            {
                ct().canceled_event=make_shared<Event>();
                if
                (
                    ct().SetConsoleCtrlHandler
                    ((PHANDLER_ROUTINE)control_key_pressed,TRUE)==FALSE
                )
                    throw runtime_error(describe
                    (
                        "function:'SetConsoleCtrlHandler':"
                        "failed(",ct().GetLastError(),")"
                    ));
                run(parse_script());
            }
        } catch(const runtime_error&error)
        {
            (*ct().err)<<describe
            ("error(",ct().index,"):",error.what())<<endl;
            result=1;
        } catch(const canceled_exception&) {}
        return result;
    }

    HWND find_target()
    {
        HWND window=ct().FindWindowW
        (NULL,multi_to_wide(ct().properties.at("target"),CP_UTF8).get());
        if(window==NULL)
            throw runtime_error(describe
            ("target:'",ct().properties.at("target"),"':not found"));
        return window;
    }

    void frame_begin(const size_t&number)
    {ct().frames.push_back({0,ct().index,number});}

    bool frame_end()
    {
        bool continue_=ct().frames.back().number==0||
            ++ct().frames.back().counter<ct().frames.back().number;
        if(continue_) ct().index=ct().frames.back().entry;
        else ct().frames.pop_back();
        return continue_;
    }

    shared_ptr<wchar_t> multi_to_wide
    (const string&multi,const UINT&codePage)
    {
        int length=MultiByteToWideChar
        (codePage,0,multi.c_str(),multi.size()+1,NULL,0);
        if(length==0)
            throw runtime_error(describe
            (
                "function:'MultiByteToWideChar':"
                "failed(",ct().GetLastError(),")"
            ));
        shared_ptr<wchar_t> wide
        (new wchar_t[length],default_delete<wchar_t[]>());
        if
        (
            MultiByteToWideChar
            (codePage,0,multi.c_str(),multi.size()+1,wide.get(),length)!=
                length
        )
            throw runtime_error(describe
            (
                "function:'MultiByteToWideChar':"
                "failed(",ct().GetLastError(),")"
            ));
        return wide;
    }

    shared_ptr<Command> new_command
    (
        const map<string,COMMAND_FACTORY>&factories,
        const shared_ptr<Row>&row,
        const size_t&index
    )
    {
        shared_ptr<Command> command;
        if(row->tokens().empty()) command=make_shared<NullCommand>(row);
        else
        {
            if(row->tokens().size()<=index)
                throw runtime_error(describe
                ("row:'",row->description(),"':few switches"));
            string switch_=row->tokens().at(index);
            string switchAsLC=to_lower_case(switch_);
            if(factories.find(switchAsLC)==factories.end())
                throw runtime_error(describe
                ("switch:'",switch_,"':unknown"));
            command=factories.at(switchAsLC)(row);
        }
        return command;
    }

    map<string,string> parse_properties(int argc,char**argv)
    {
        map<string,string> properties;
        for(size_t i=1;i<argc;i++)
        {
            string argument=argv[i];
            string::size_type eqpos=argument.find('=');
            string name,value;
            if(eqpos==string::npos) name=argument;
            else
            {
                name=to_lower_case(argument.substr(0,eqpos));
                value=argument.substr(eqpos+1);
            }
            properties.insert(make_pair(name,value));
        }
        return properties;
    }

    vector<shared_ptr<Command>> parse_script()
    {
        static map<string,COMMAND_FACTORY> keyCommandFactories(
        {
            {
                "down",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(row);}
            },
            {
                "press",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyPressCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(row);}
            },
        });
        static map<string,COMMAND_FACTORY> mouseButtonCommandFactories(
        {
            {
                "click",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<MouseButtonClickCommand>(row);}
            },
            {
                "doubleclick",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<MouseButtonDoubleClickCommand>(row);}
            },
            {
                "down",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<MouseButtonDownCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<MouseButtonUpCommand>(row);}
            },
        });
        static map<string,COMMAND_FACTORY> mouseCommandFactories(
        {
            {
                "left",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(mouseButtonCommandFactories,row,2);}
            },
            {
                "middle",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(mouseButtonCommandFactories,row,2);}
            },
            {
                "move",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<MouseMoveCommand>(row);}
            },
            {
                "right",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(mouseButtonCommandFactories,row,2);}
            },
            {
                "wheel",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<MouseWheelCommand>(row);}
            },
        });
        static map<string,COMMAND_FACTORY> loopCommandFactories(
        {
            {
                "begin",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<LoopBeginCommand>(row);}
            },
            {
                "end",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<LoopEndCommand>(row);}
            },
        });
        static map<string,COMMAND_FACTORY> commandFactories(
        {
            {
                "key",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(keyCommandFactories,row,1);}
            },
            {
                "loop",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(loopCommandFactories,row,1);}
            },
            {
                "mouse",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(mouseCommandFactories,row,1);}
            },
            {
                "sleep",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<SleepCommand>(row);}
            },
        });
        vector<shared_ptr<Command>> commands;
        string description;
        for(ct().index=0;getline(*ct().in,description);ct().index++)
        {
            auto row=make_shared<Row>(chomp_cr(description));
            commands.push_back(new_command(commandFactories,row,0));
        }
        return commands;
    }

    void run(const vector<shared_ptr<Command>>&commands)
    {
        if(ct().properties.find("ready")!=ct().properties.end())
             wait(to_number(ct().properties.at("ready")));
        ct().index=0;
        size_t numberOfRepetities=1;
        if(ct().properties.find("repeat")!=ct().properties.end())
             numberOfRepetities=to_number(ct().properties.at("repeat"));
        frame_begin(numberOfRepetities);
        do
        {
            while(ct().index<commands.size())
            {
                if(ct().properties.find("target")!=ct().properties.end())
                {
                    while(find_target()!=ct().GetForegroundWindow())
                        wait(ct().GetDoubleClickTime());
                }
                auto command=commands.at(ct().index);
                if(ct().properties.find("silent")==ct().properties.end())
                    (*ct().out)<<command->row()->description()<<endl;
                command->execute();
            }
            if(ct().frames.size()!=1)
            {
                ct().index=ct().frames.back().entry-1;
                throw runtime_error(describe
                ("loop begin:no corresponding end"));
            }
        } while(frame_end());
    }

    int system_metrics(const int&index)
    {
        int result=ct().GetSystemMetrics(index);
        if(result==0)
            throw runtime_error(describe
            ("function:'GetSystemMetrics':failed"));
        return result;
    }

    string to_lower_case(const string&from)
    {
        string to;
        to.resize(from.size());
        transform(from.begin(),from.end(),to.begin(),::tolower);
        return to;
    }

    long to_number(const string&from)
    {
        char*end;
        long to=strtol(from.c_str(),&end,0);
        if(*end)
            throw runtime_error(describe
            ("number:'",from,"':invalid format"));
        return to;
    }

    vector<string> tokenize(const string&str,const string&delimiters)
    {
        vector<string> tokens;
        for(string::size_type pos=0;pos<=str.length();)
        {
            string::size_type dlmpos=str.find_first_of(delimiters,pos);
            if(dlmpos==string::npos) dlmpos=str.length();
            if(dlmpos!=pos) tokens.push_back(str.substr(pos,dlmpos-pos));
            pos=dlmpos+1;
        }
        return tokens;
    }

    void wait(const DWORD&time)
    {
        DWORD resultOfW4SO=ct().WaitForSingleObject
        (ct().canceled_event->handle(),time);
        if(resultOfW4SO==WAIT_FAILED)
            throw runtime_error(describe
            (
                "function:'WaitForSingleObject':"
                "failed(",ct().GetLastError(),")"
            ));
        if(resultOfW4SO==WAIT_OBJECT_0) throw canceled_exception();
    }
}
