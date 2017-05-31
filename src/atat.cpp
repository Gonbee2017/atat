#include<algorithm>
#include"atat.h"
#include<cstdlib>

namespace atat
{
    Command::Command(const shared_ptr<Row>&row_):row_(row_) {}

    Row*Command::row() {return row_.get();}

    Context::Context():frames_({{0,0,0}}),index_(0)
    {
        abortedEvent_=make_shared<SystemObject>
        (atat::CreateEvent(NULL,TRUE,FALSE,NULL));
        if(abortedEvent_->handle()==NULL)
            throw runtime_error(describe
            (
                "function:'CreateEvent':failed(",
                GetLastError(),
                ")"
            ));
    }

    const HANDLE&Context::abortedEvent() {return abortedEvent_->handle();}

    vector<frame>&Context::frames() {return frames_;}

    size_t&Context::index() {return index_;}

    shared_ptr<Context>&Context::instance() {return instance_;}

    map<string,string>&Context::properties() {return properties_;}

    shared_ptr<Context> Context::instance_;

    KeyCommand::KeyCommand(const shared_ptr<Row>&row_):Command(row_)
    {
        if(row_->tokens().size()-1!=2)
            throw runtime_error(describe
            (
                "row:'",
                row_->description(),
                "':wrong number of tokens"
            ));
        string key=row_->tokens().at(2);
        static map<string,WORD> keyMap(
        {
            {lower_case("ESCAPE"),      0x01},
            {lower_case("1"),           0x02},
            {lower_case("2"),           0x03},
            {lower_case("3"),           0x04},
            {lower_case("4"),           0x05},
            {lower_case("5"),           0x06},
            {lower_case("6"),           0x07},
            {lower_case("7"),           0x08},
            {lower_case("8"),           0x09},
            {lower_case("9"),           0x0A},
            {lower_case("0"),           0x0B},
            {lower_case("MINUS"),       0x0C},
            {lower_case("EQUALS"),      0x0D},
            {lower_case("BACK"),        0x0E},
            {lower_case("TAB"),         0x0F},
            {lower_case("Q"),           0x10},
            {lower_case("W"),           0x11},
            {lower_case("E"),           0x12},
            {lower_case("R"),           0x13},
            {lower_case("T"),           0x14},
            {lower_case("Y"),           0x15},
            {lower_case("U"),           0x16},
            {lower_case("I"),           0x17},
            {lower_case("O"),           0x18},
            {lower_case("P"),           0x19},
            {lower_case("LBRACKET"),    0x1A},
            {lower_case("RBRACKET"),    0x1B},
            {lower_case("RETURN"),      0x1C},
            {lower_case("LContol"),     0x1D},
            {lower_case("A"),           0x1E},
            {lower_case("S"),           0x1F},
            {lower_case("D"),           0x20},
            {lower_case("F"),           0x21},
            {lower_case("G"),           0x22},
            {lower_case("H"),           0x23},
            {lower_case("J"),           0x24},
            {lower_case("K"),           0x25},
            {lower_case("L"),           0x26},
            {lower_case("SEMICOLON"),   0x27},
            {lower_case("APOSTROPHE"),  0x28},
            {lower_case("GRAVE"),       0x29},
            {lower_case("LSHIFT"),      0x2A},
            {lower_case("BACKSLASH"),   0x2B},
            {lower_case("Z"),           0x2C},
            {lower_case("X"),           0x2D},
            {lower_case("C"),           0x2E},
            {lower_case("V"),           0x2F},
            {lower_case("B"),           0x30},
            {lower_case("N"),           0x31},
            {lower_case("M"),           0x32},
            {lower_case("COMMA"),       0x33},
            {lower_case("PERIOD"),      0x34},
            {lower_case("SLASH"),       0x35},
            {lower_case("RSHIFT"),      0x36},
            {lower_case("MULTIPLY"),    0x37},
            {lower_case("LMENU"),       0x38},
            {lower_case("SPACE"),       0x39},
            {lower_case("CAPITAL"),     0x3A},
            {lower_case("F1"),          0x3B},
            {lower_case("F2"),          0x3C},
            {lower_case("F3"),          0x3D},
            {lower_case("F4"),          0x3E},
            {lower_case("F5"),          0x3F},
            {lower_case("F6"),          0x40},
            {lower_case("F7"),          0x41},
            {lower_case("F8"),          0x42},
            {lower_case("F9"),          0x43},
            {lower_case("F10"),         0x44},
            {lower_case("NUMLOCK"),     0x45},
            {lower_case("SCROLL"),      0x46},
            {lower_case("NUMPAD7"),     0x47},
            {lower_case("NUMPAD8"),     0x48},
            {lower_case("NUMPAD9"),     0x49},
            {lower_case("SUBTRACT"),    0x4A},
            {lower_case("NUMPAD4"),     0x4B},
            {lower_case("NUMPAD5"),     0x4C},
            {lower_case("NUMPAD6"),     0x4D},
            {lower_case("ADD"),         0x4E},
            {lower_case("NUMPAD1"),     0x4F},
            {lower_case("NUMPAD2"),     0x50},
            {lower_case("NUMPAD3"),     0x51},
            {lower_case("NUMPAD0"),     0x52},
            {lower_case("DECIMAL"),     0x53},
            {lower_case("F11"),         0x57},
            {lower_case("F12"),         0x58},
            {lower_case("F13"),         0x64},
            {lower_case("F14"),         0x65},
            {lower_case("F15"),         0x66},
            {lower_case("KANA"),        0x70},
            {lower_case("CONVERT"),     0x79},
            {lower_case("NOCONVERT"),   0x7B},
            {lower_case("YEN"),         0x7D},
            {lower_case("NUMPADEQUALS"),0x8D},
            {lower_case("CIRCUMFLEX"),  0x90},
            {lower_case("AT"),          0x91},
            {lower_case("COLON"),       0x92},
            {lower_case("UNDERrow"),   0x93},
            {lower_case("KANJI"),       0x94},
            {lower_case("STOP"),        0x95},
            {lower_case("AX"),          0x96},
            {lower_case("UNLABELED"),   0x97},
            {lower_case("NUMPADENTER"), 0x9C},
            {lower_case("RCONTROL"),    0x9D},
            {lower_case("NUMPADCOMMA"), 0xB3},
            {lower_case("DIVIDE"),      0xB5},
            {lower_case("SYSRQ"),       0xB7},
            {lower_case("RMENU"),       0xB8},
            {lower_case("PAUSE"),       0xC5},
            {lower_case("HOME"),        0xC7},
            {lower_case("UP"),          0xC8},
            {lower_case("PRIOR"),       0xC9},
            {lower_case("LEFT"),        0xCB},
            {lower_case("RIGHT"),       0xCD},
            {lower_case("END"),         0xCF},
            {lower_case("DOWN"),        0xD0},
            {lower_case("NEXT"),        0xD1},
            {lower_case("INSERT"),      0xD2},
            {lower_case("DELETE"),      0xD3},
            {lower_case("LWIN"),        0xDB},
            {lower_case("RWIN"),        0xDC},
            {lower_case("APPS"),        0xDD},
            {lower_case("POWER"),       0xDE},
            {lower_case("SLEEP"),       0xDF},
        });
        if(keyMap.find(lower_case(key))==keyMap.end())
            throw runtime_error(describe
            (
                "key:'",
                key,
                "':unknown"
            ));
        code_=keyMap.at(lower_case(key));
    }

    void KeyCommand::send(const WORD&code,const DWORD&up)
    {
        INPUT input;
        input.type=INPUT_KEYBOARD;
        input.ki.wScan=code;
        input.ki.dwFlags=KEYEVENTF_SCANCODE|up;
        input.ki.time=0;
        input.ki.dwExtraInfo=0;
        if(atat::SendInput(1,&input,sizeof(INPUT))!=1)
            throw runtime_error(describe
            (
                "function:'SendInput':failed(",
                GetLastError(),
                ")"
            ));
    }

    KeyDownCommand::KeyDownCommand(const shared_ptr<Row>&row_):
        KeyCommand(row_) {}

    void KeyDownCommand::execute()
    {
        send(code_,0);
        wait(atat::GetDoubleClickTime()/2);
        Context::instance()->index()++;
    }

    KeyPressCommand::KeyPressCommand(const shared_ptr<Row>&row_):
        KeyCommand(row_) {}

    void KeyPressCommand::execute()
    {
        send(code_,0);
        wait(atat::GetDoubleClickTime()/2);
        send(code_,KEYEVENTF_KEYUP);
        wait(atat::GetDoubleClickTime()/2);
        Context::instance()->index()++;
    }

    KeyUpCommand::KeyUpCommand(const shared_ptr<Row>&row_):KeyCommand(row_) {}

    void KeyUpCommand::execute()
    {
        send(code_,KEYEVENTF_KEYUP);
        wait(atat::GetDoubleClickTime()/2);
        Context::instance()->index()++;
    }

    Row::Row(const string&description_):description_(description_)
    {
        string::size_type shrpos=description_.find_first_of('#');
        if(shrpos==string::npos) shrpos=description_.length();
        tokens_=tokenize(description_.substr(0,shrpos)," \t");
    }

    const string&Row::description() {return description_;}

    const vector<string>&Row::tokens() {return tokens_;}

    MouseButtonClickCommand::MouseButtonClickCommand
    (const shared_ptr<Row>&row_):MouseButtonCommand(row_) {}

    void MouseButtonClickCommand::execute()
    {
        send(0,0,0,button_);
        wait(atat::GetDoubleClickTime()/2);
        send(0,0,0,button_<<1);
        wait(atat::GetDoubleClickTime()/2);
        Context::instance()->index()++;
    }

    MouseButtonCommand::MouseButtonCommand(const shared_ptr<Row>&row_):
        MouseCommand(row_)
    {
        if(row_->tokens().size()!=3)
            throw runtime_error(describe
            (
                "row:'",
                row_->description(),
                "':wrong number of tokens"
            ));
        static map<string,DWORD> button_map(
        {
            {"left",  MOUSEEVENTF_LEFTDOWN},
            {"right", MOUSEEVENTF_RIGHTDOWN},
            {"middle",MOUSEEVENTF_MIDDLEDOWN},
        });
        button_=button_map.at(row_->tokens().at(1));
    }

    MouseButtonDoubleClickCommand::MouseButtonDoubleClickCommand
    (const shared_ptr<Row>&row_):MouseButtonCommand(row_) {}

    void MouseButtonDoubleClickCommand::execute()
    {
        send(0,0,0,button_);
        wait(atat::GetDoubleClickTime()/4);
        send(0,0,0,button_<<1);
        wait(atat::GetDoubleClickTime()/4);
        send(0,0,0,button_);
        wait(atat::GetDoubleClickTime()/4);
        send(0,0,0,button_<<1);
        wait(atat::GetDoubleClickTime()/4);
        Context::instance()->index()++;
    }

    MouseButtonDownCommand::MouseButtonDownCommand
    (const shared_ptr<Row>&row_):MouseButtonCommand(row_) {}

    void MouseButtonDownCommand::execute()
    {
        send(0,0,0,button_);
        wait(atat::GetDoubleClickTime()/2);
        Context::instance()->index()++;
    }

    MouseButtonUpCommand::MouseButtonUpCommand(const shared_ptr<Row>&row_):
        MouseButtonCommand(row_) {}

    void MouseButtonUpCommand::execute()
    {
        send(0,0,0,button_<<1);
        wait(atat::GetDoubleClickTime()/2);
        Context::instance()->index()++;
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
        if(atat::SendInput(1,&input,sizeof(INPUT))!=1)
            throw runtime_error(describe
            (
                "function:'SendInput':failed(",
                GetLastError(),
                ")"
            ));
    }

    MouseMoveCommand::MouseMoveCommand(const shared_ptr<Row>&row_):
        MouseCommand(row_)
    {
        if(row_->tokens().size()!=4)
            throw runtime_error(describe
            (
                "row:'",
                row_->description(),
                "':wrong number of tokens"
            ));
        x_=atoi(row_->tokens().at(2).c_str());
        y_=atoi(row_->tokens().at(3).c_str());
    }

    void MouseMoveCommand::execute()
    {
        RECT targetRect;
        targetRect.left=targetRect.top=0;
        if(properties().find("target")!=properties().end())
        {
            if(atat::GetWindowRect(find_target(),&targetRect)==FALSE)
                throw runtime_error(describe
                (
                    "function:'GetWindowRect':failed(",
                    GetLastError(),
                    ")"
                ));
        }
        int screenWidth=atat::GetSystemMetrics(SM_CXSCREEN);
        if(screenWidth==0)
            throw runtime_error(describe
            ("function:'GetSystemMetrics':failed"));
        int screenHeight=atat::GetSystemMetrics(SM_CYSCREEN);
        if(screenHeight==0)
            throw runtime_error(describe
            ("function:'GetSystemMetrics':failed"));
        LONG mouseX=((x_+targetRect.left)<<16)/screenWidth;
        LONG mouseY=((y_+targetRect.top)<<16)/screenHeight;
        send(mouseX,mouseY,0,MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE);
        Context::instance()->index()++;
    }

    MouseWheelCommand::MouseWheelCommand(const shared_ptr<Row>&row_):
        MouseCommand(row_)
    {
        if(row_->tokens().size()!=3)
            throw runtime_error(describe
            (
                "row:'",
                row_->description(),
                "':wrong number of tokens"
            ));
        amount_=atoi(row_->tokens().at(2).c_str());
    }

    void MouseWheelCommand::execute()
    {
        send(0,0,(DWORD)amount_,MOUSEEVENTF_WHEEL);
        Context::instance()->index()++;
    }

    NullCommand::NullCommand(const shared_ptr<Row>&row_):
        Command(row_){}

    void NullCommand::execute() {Context::instance()->index()++;}

    LoopBeginCommand::LoopBeginCommand(const shared_ptr<Row>&row_):
        Command(row_)
    {
        if(row_->tokens().size()-1>2)
            throw runtime_error(describe
            (
                "row:'",
                row_->description(),
                "':wrong number of tokens"
            ));
        if(row_->tokens().size()-1==2)
            number_=atoi(row_->tokens().at(2).c_str());
        else number_=0;
    }

    void LoopBeginCommand::execute()
    {
        Context::instance()->frames().push_back
        ({0,++Context::instance()->index(),number_});
    }

    LoopEndCommand::LoopEndCommand(const shared_ptr<Row>&row_):Command(row_)
    {
        if(row_->tokens().size()-1!=1)
            throw runtime_error(describe
            (
                "row:'",
                row_->description(),
                "':wrong number of tokens"
            ));
    }

    void LoopEndCommand::execute()
    {
        vector<frame>&frames=Context::instance()->frames();
        if
        (
            frames.back().number==0||
            ++frames.back().counter<frames.back().number
        ) Context::instance()->index()=frames.back().entry;
        else
        {
            Context::instance()->index()++;
            frames.pop_back();
        }
    }

    SleepCommand::SleepCommand(const shared_ptr<Row>&row_):Command(row_)
    {
        if(row_->tokens().size()-1!=1)
            throw runtime_error(describe
            (
                "row:'",
                row_->description(),
                "':wrong number of tokens"
            ));
        time_=atoi(row_->tokens().at(1).c_str());
    }

    void SleepCommand::execute()
    {
        wait(time_);
        Context::instance()->index()++;
    }

    SystemObject::SystemObject(HANDLE handle_):handle_(handle_) {}

    SystemObject::~SystemObject()
    {if(handle_!=NULL) atat::CloseHandle(handle_);}

    const HANDLE&SystemObject::handle() {return handle_;}

    BOOL control_key_pressed(DWORD type)
    {
        BOOL handled=FALSE;
        if(type==CTRL_C_EVENT)
        {
            atat::SetEvent(Context::instance()->abortedEvent());
            handled=TRUE;
        }
        return handled;
    }

    HWND find_target()
    {
        if(properties().find("target")==properties().end())
            throw runtime_error(describe
            ("property:'target':not found"));
        HWND window=atat::FindWindowW
        (NULL,multi_to_wide(properties().at("target"),CP_UTF8).get());
        if(window==NULL)
            throw runtime_error(describe
            (
                "target:'",
                properties().at("target"),
                "':not found"
            ));
        return window;
    }

    string lower_case(const string&source)
    {
        string destination;
        destination.resize(source.size());
        transform
        (
            source.begin(),
            source.end(),
            destination.begin(),
            ::tolower
        );
        return destination;
    }

    shared_ptr<wchar_t> multi_to_wide
    (const string&str,const UINT&codePage)
    {
        int length=MultiByteToWideChar
        (
            codePage,
            0,
            str.c_str(),
            str.size()+1,
            NULL,
            0
        );
        if(length==0)
            throw runtime_error(describe
            (
                "function:'MultiByteToWideChar':failed(",
                GetLastError(),
                ")"
            ));
        shared_ptr<wchar_t> utf16
        (new wchar_t[length],default_delete<wchar_t[]>());
        if
        (
            MultiByteToWideChar
            (
                codePage,
                0,
                str.c_str(),
                str.size()+1,
                utf16.get(),
                length
            )!=length
        )
            throw runtime_error(describe
            (
                "function:'MultiByteToWideChar':failed(",
                GetLastError(),
                ")"
            ));
        return utf16;
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
                (
                    "row:'",
                    row->description(),
                    "':few switches"
                ));
            string switch_=lower_case(row->tokens().at(index));
            if(factories.find(switch_)==factories.end())
                throw runtime_error(describe
                (
                    "switch:'",
                    switch_,
                    "':unknown"
                ));
            command=factories.at(switch_)(row);
        }
        return command;
    }

    map<string,string> parse_properties(int argc,char**argv)
    {
        map<string,string> properties_;
         for(size_t i=1;i<argc;i++)
         {
             string argument=argv[i];
             string::size_type eqpos=argument.find('=');
             string name,value;
             if(eqpos==string::npos) name=argument;
             else
             {
                 name=lower_case(argument.substr(0,eqpos));
                 value=argument.substr(eqpos+1);
             }
             properties_.insert(make_pair(name,value));
         }
         return properties_;
    }

    vector<shared_ptr<Command>> parse_script(istream&is)
    {
        static map<string,COMMAND_FACTORY> keyCommandFactories(
        {
            {
                "down",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(row);}
            },
            {
                "press",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyPressCommand>(row);}
            },
        });
        static map<string,COMMAND_FACTORY> mouseButtonCommandFactories(
        {
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
        });
        static map<string,COMMAND_FACTORY> mouseCommandFactories(
        {
            {
                "move",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<MouseMoveCommand>(row);}
            },
            {
                "wheel",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<MouseWheelCommand>(row);}
            },
            {
                "left",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(mouseButtonCommandFactories,row,2);}
            },
            {
                "right",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(mouseButtonCommandFactories,row,2);}
            },
            {
                "middle",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(mouseButtonCommandFactories,row,2);}
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
                "mouse",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(mouseCommandFactories,row,1);}
            },
            {
                "loop",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return new_command(loopCommandFactories,row,1);}
            },
            {
                "sleep",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<SleepCommand>(row);}
            },
        });
        vector<shared_ptr<Command>> commands;
        string description;
        for
        (
            Context::instance()->index()=0;
            getline(is,description);
            Context::instance()->index()++
        )
        {
            auto row=make_shared<Row>(description);
            commands.push_back(new_command(commandFactories,row,0));
        }
        return commands;
    }

    map<string,string>&properties()
    {return Context::instance()->properties();}

    int run(int argc,char**argv,istream&in,ostream&out,ostream&err)
    {
        int result=0;
        try
        {
            Context::instance()=make_shared<Context>();
            properties()=parse_properties(argc,argv);
            if(properties().find("help")!=properties().end())
            {
                out<<describe
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
"    Specify a name, if it has a value, specify it following the equals.\n"
"    In the following list, a name is indicated in lowercase,\n"
"    and a value is indicated in uppercase.\n"
"\n"
"    help\n"
"        Show this document, and finish without doing anything.\n"
"    silent\n"
"        Don't show the commands to be executed.\n"
"    target=CAPTION\n"
"        CAPTION is the caption of the target window.\n"
"        Execute the command when this window is active.\n"
"        Also, specify the relative coordinates of this window\n"
"        during 'mouse move' command.\n"
"        If it's not specified, execute the command any window active.\n"
"\n"
"COMMANDS\n"
"    Command consists of switches followed by parameters.\n"
"    Separete switches and parameters with spaces or tabs.\n"
"    In the following list, switches are indicated in lowercase,\n"
"    and parameters are indicated in uppercase.\n"
"    Also, if there are multiple choices on the switch,\n"
"    separate them with vertical bars.\n"
"\n"
"    key down|up|press KEY\n"
"        Down or up or both the key on the keyboard.\n"
"        KEY is the key to be operated.\n"
"        Specify a string following 'DIK_'.\n"
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
"REMARKS\n"
"    Script can contains comments, for example:\n"
"        # Script01 to earn experience.\n"
"\n"
"        # Wait for ready.\n"
"        sleep 3000\n"
"\n"
"        # Repeat infinitely.\n"
"        loop begin\n"
"            key press TAB # Target an enemy.\n"
"            key press 1   # Attack the target.\n"
"        loop end\n"
"    The string following the '#' is treated as a comment.\n"
"    Also, empty rows just skip.\n"
                );
            } else
            {
                if
                (
                    atat::SetConsoleCtrlHandler
                    ((PHANDLER_ROUTINE)control_key_pressed,TRUE)==FALSE
                )
                    throw runtime_error(describe
                    (
                        "function:'SetConsoleCtrlHandler':failed(",
                        GetLastError(),
                        ")"
                    ));
                vector<shared_ptr<Command>> commands=parse_script(in);
                for
                (
                    Context::instance()->index()=0;
                    Context::instance()->index()<commands.size();
                )
                {
                    wait_active();
                    auto command=commands.at(Context::instance()->index());
                    if(properties().find("silent")==properties().end())
                        out<<command->row()->description()<<endl;
                    command->execute();
                }
            }
        } catch(const runtime_error&error)
        {
            err<<describe
            (
                "error(",
                Context::instance()->index(),
                "):",
                error.what()
            )<<endl;
            result=1;
        } catch(const aborted_exception&) {}
        return result;
    }

    void setup_io()
    {
        atat::CloseHandle=::CloseHandle;
        atat::CreateEvent=::CreateEvent;
        atat::FindWindowW=::FindWindowW;
        atat::GetDoubleClickTime=::GetDoubleClickTime;
        atat::GetForegroundWindow=::GetForegroundWindow;
        atat::GetLastError=::GetLastError;
        atat::GetSystemMetrics=::GetSystemMetrics;
        atat::GetWindowRect=::GetWindowRect;
        atat::SendInput=::SendInput;
        atat::SetConsoleCtrlHandler=::SetConsoleCtrlHandler;
        atat::SetEvent=::SetEvent;
        atat::WaitForSingleObject=::WaitForSingleObject;
    }

    vector<string> tokenize(const string&str,const string&delimiters)
    {
        vector<string> tokens;
        for(string::size_type delpos=0;delpos<=str.length();)
        {
            string::size_type found=str.find_first_of(delimiters,delpos);
            if(found==string::npos) found=str.length();
            if(found!=delpos)
                tokens.push_back(str.substr(delpos,found-delpos));
            delpos=found+1;
        }
        return tokens;
    }

    void wait(const DWORD&time)
    {
        DWORD waitResult=atat::WaitForSingleObject
        (Context::instance()->abortedEvent(),time);
        if(waitResult==WAIT_FAILED)
            throw runtime_error(describe
            (
                "function:'WaitForSingleObject':failed(",
                GetLastError(),
                ")"
            ));
        if(waitResult==WAIT_OBJECT_0) throw aborted_exception();
    }

    void wait_active()
    {
        if(properties().find("target")!=properties().end())
        {
            while(find_target()!=atat::GetForegroundWindow())
                wait(atat::GetDoubleClickTime());
        }
    }

    function<BOOL(HANDLE)> CloseHandle;
    function<HANDLE(LPSECURITY_ATTRIBUTES,BOOL,BOOL,LPCTSTR)> CreateEvent;
    function<HWND(const wchar_t*,const wchar_t*)> FindWindowW;
    function<UINT()> GetDoubleClickTime;
    function<HWND()> GetForegroundWindow;
    function<DWORD()> GetLastError;
    function<int(int)> GetSystemMetrics;
    function<BOOL(HWND,LPRECT)> GetWindowRect;
    function<UINT(UINT,LPINPUT,int)> SendInput;
    function<BOOL(PHANDLER_ROUTINE,BOOL)> SetConsoleCtrlHandler;
    function<BOOL(HANDLE)> SetEvent;
    function<DWORD(HANDLE,DWORD)> WaitForSingleObject;
}
