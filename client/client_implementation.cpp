#include "../Templates/template.h"
#include <sys/socket.h>

// inet_addr
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <iomanip>
#include <vector>
#include <algorithm>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>
#define PORT 8080
using namespace std;


struct StudentPerformance {
    string id;
    map<string, double> topicAccuracies;
};

void displayPerformanceChart(const map<string, StudentPerformance>& studentData) {
    // Terminal width settings
    const int nameWidth = 15;
    const int barWidth = 40;
    
    // Print header
    cout << "\n============= Student Topic Performance =============\n\n";
    
    // For each student
    for (const auto& [studentId, performance] : studentData) {
        cout << "\nStudent ID: " << studentId << "\n";
        cout << string(50, '-') << "\n";
        
        // Calculate max accuracy for scaling
        double maxAccuracy = 0;
        for (const auto& [topic, accuracy] : performance.topicAccuracies) {
            maxAccuracy = max(maxAccuracy, accuracy);
        }
        
        // Display each topic performance
        for (const auto& [topic, accuracy] : performance.topicAccuracies) {
            // Topic name
            cout << left << setw(nameWidth) << topic.substr(0, nameWidth-1) << " | ";
            
            // Calculate bar length
            int numBars = (int)((accuracy / 100.0) * barWidth);
            
            // Draw bar using standard ASCII
            string bar = string(numBars, '#') + string(barWidth - numBars, '.');
            
            // Add color based on performance (optional)
            cout << bar << " | " << fixed << setprecision(1) << accuracy << "%";
            
            // Add a simple indicator
            if (accuracy == 100.0) cout << " ★";
            else if (accuracy >= 75.0) cout << " ✓";
            
            cout << "\n";
        }
        cout << "\n";
    }
    cout << "================================================\n\n";
}


bool validUsertype(char &userType)
{
    return (userType == 'S' || userType == 'T');
}

bool isValidAnswer(string answer)
{
    return (answer[0]>='a' and answer[0]<='d');
}

User::User(string username, string password, string usertype)
{
    this->username = username;
    this->password = password;
    this->usertype = usertype;
}

User::User(string password)
{
    this->password = password;
}

void User::user_specific_functions(int client_socket)
{
    cout << "Overrideable function \n";
    return;
}

string User::getUsername()
{
    return username;
}

Student::Student(string username, string password, string usertype, string rollno, string department) : User(username, password, usertype)
{
    this->rollno = rollno;
    this->department = department;
}

Student::Student(string id, string dept, string password) : User(password)
{
    this->rollno = id;
    this->department = dept;
}

Teacher::Teacher(string username, string password, string usertype, string id, string department) : User(username, password, usertype)
{
    this->teacherid = id;
    this->department = department;
}

Teacher::Teacher(string id, string dept, string password) : User(password)
{
    this->teacherid = id;
    this->department = dept;
}

Client::Client()
{
    // Create a stream socket
    this->client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Initialise port number and address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    // inet_addr("192.168.1.100");
    server_address.sin_addr.s_addr = INADDR_ANY; // this can be changed for remote server but for local this is good enough
    server_address.sin_port = htons(PORT);

    // Initiate a socket connection
    int connection_status = connect(this->client_socket, (struct sockaddr *)&(server_address), sizeof(server_address));

    // Check for connection error
    if (connection_status < 0)
    {
        perror("Error\n");
        return;
    }
    //rules
    // a teacher can see any department question and a student can see only his department questions
    // a teacher can see any department result and a student can see only his department result
    // a teacher can set questions for his/her department and a student can take exam for his/her department only which is being regulated by ID so your id should contain your branch name abbreviation

    printf("Connection established\n");

    // Send data to the socket
    int choice, code;
    char userType;
    system("clear");
    while (1)
    {
        cout << "========================================\n";
        cout << "      Welcome to the Exam Center\n";
        cout << "========================================\n";
        cout << "Are you a student or a teacher?(S for Student, T for Teacher) \n";
        cin >> userType;
        if(validUsertype(userType))
        {
            break;
        }
        else
        {
            cout<<"\nInvalid input :( . Enter again..\n\n";
        }
    }
    cout << "========================================\n";
    cout << "Enter your choice:\n";
    cout << " 1) Register\n";
    cout << " 2) Login\n";
    cout << " 3) Exit\n";
    cout << "========================================\n";
    cin >> choice;
    switch (choice)
    {
    case 1:
    {
        code = REGISTRATION_CODE;
        send(this->client_socket, &code, sizeof(code), 0);
        registeruser(userType);
        break;
    }
    case 2:
    {
        code = LOGIN_CODE;
        send(this->client_socket, &code, sizeof(code), 0);
        login(userType);
        break;
    }
    case 3:
    {
        // Close the connection
        cout << "Exiting!! :-(" << endl;
        close(this->client_socket);
        exit(0);
    }
    }
}

void Client::registeruser(char &usertype)
{
    send(client_socket, &usertype, sizeof(usertype), 0); // to call specific registration func

    int code;
    string id, department;

    ClientUserInfo *clientInfo = new ClientUserInfo;
    if (usertype == 'S')
    {
        StudentUserInfo *newUserInfo = new StudentUserInfo;
        cout << student_register_menu;
        cin >> newUserInfo->username;
        cin >> newUserInfo->password;
        cin >> newUserInfo->rollno;
        cin >> newUserInfo->department;

        send(client_socket, newUserInfo, sizeof(*newUserInfo), 0);

        clientInfo->username = newUserInfo->username;
        clientInfo->password = newUserInfo->password;
        clientInfo->usertype = "S";
        id = newUserInfo->rollno;
        department = newUserInfo->department;
    }
    else
    {
        TeacherUserInfo *newUserInfo = new TeacherUserInfo;
        cout << teacher_register_menu;
        cin >> newUserInfo->username;
        cin >> newUserInfo->password;
        cin >> newUserInfo->teacherid;
        cin >> newUserInfo->department;

        send(client_socket, newUserInfo, sizeof(*newUserInfo), 0);

        clientInfo->username = newUserInfo->username;
        clientInfo->password = newUserInfo->password;
        clientInfo->usertype = "T";
        id = newUserInfo->teacherid;
        department = newUserInfo->department;
    }

    recv(client_socket, &code, sizeof(code), 0);

    if (code == 200)
    {
        cout << "========================================\n";
        cout << "Welcome, " << clientInfo->username << ", to Exam center!!\n";
        cout << "========================================\n";

        if (usertype == 'S')
        {
            this->client = new Student(clientInfo->username, clientInfo->password, clientInfo->usertype, id, department);
            Student *student = dynamic_cast<Student *>(this->client);
            if (student == NULL)
            {
                cout << "downcasting failed\n"
                     << endl;
                code = END_CONNECTION_CODE;
                send(client_socket, &code, sizeof(code), 0);
                close(this->client_socket);
                exit(0);
            }
            student->user_specific_functions(client_socket);
        }
        else
        {
            this->client = new Teacher(clientInfo->username, clientInfo->password, clientInfo->usertype, id, department);
            Teacher *teacher = dynamic_cast<Teacher *>(this->client);
            if (teacher == NULL)
            {
                cout << "downcasting failed\n"
                     << endl;
                code = END_CONNECTION_CODE;
                send(client_socket, &code, sizeof(code), 0);
                close(this->client_socket);
                exit(0);
            }
            teacher->user_specific_functions(client_socket);
        }
    }
    else
    {
        cout << "Registration Failed!" << endl;
    }
    close(this->client_socket);
    exit(1);
}

void Client::login(char &usertype)
{
    send(client_socket, &usertype, sizeof(usertype), 0); // to call specific login func
    int code;

    loginInfo *userInfo = new loginInfo;

    while (1)
    {
        cout << login_menu;
        cin >> userInfo->id;
        cin >> userInfo->password;

        send(client_socket, userInfo, sizeof(*userInfo), 0);

        recv(client_socket, &code, sizeof(code), 0);

        if (code == SUCCESSFUL_CODE)
        {
            cout << "========================================\n";
            cout << "Welcome to Exam center!!\n";
            cout << "========================================\n";
            string department = parseDepartment(userInfo->id);
            if (usertype == 'S')
            {
                this->client = new Student(userInfo->id, department, userInfo->password);
                Student *student = dynamic_cast<Student *>(this->client);
                student->user_specific_functions(client_socket);
            }
            else
            {
                this->client = new Teacher(userInfo->id, department, userInfo->password);
                Teacher *teacher = dynamic_cast<Teacher *>(this->client);
                teacher->user_specific_functions(client_socket);
            }
        }
        else if (code == LOGIN_FAIL_CODE)
        {
            cout << "Invalid credentials..";
        }
        else if (code == SERVER_ERROR_CODE)
        {
            cout << "Internal Server Error. Try again later\n";
            break;
        }
    }
    close(this->client_socket);
    exit(1);
}

void Student::user_specific_functions(int client_socket)
{
    while (1)
    {
        int code;
        int ch;
        cout << "========================================\n";
        cout << " 1) Start Exam\n";
        cout << " 2) See Leaderboard\n";
        cout << " 3) Exit\n";
        cout << "========================================\n";
        cin >> ch;
        bool endflag = false;
        switch (ch)
        {
        case 1:
        {
            code = START_EXAM_CODE;
            send(client_socket, &code, sizeof(code), 0);
            char dept[10];
            strcpy(dept, this->department.c_str());
            send(client_socket, &dept, sizeof(dept), 0);
            char id[100];
            cout << "Enter your roll number: \n";
            cin.ignore();
            cin.getline(id, 100);
            send(client_socket, &id, sizeof(id), 0);
            while (1)
            {
                recv(client_socket, &code, sizeof(code), 0);
                if (code == RECIEVE_QUESTION_CODE)
                {
                    StudentQuestion *question = new StudentQuestion;
                    recv(client_socket, question, sizeof(*question), 0);
                    cout << question->que << endl;
                    cout << "a: " << question->opt1 << endl;
                    cout << "b: " << question->opt2 << endl;
                    cout << "c: " << question->opt3 << endl;
                    cout << "d: " << question->opt4 << endl;
                    cout << "Marks: " << question->marks << endl;
                    char answer[5];
                    while(1)
                    {
                        cout << "Enter your answer (a/b/c/d) :" << endl;
                        cin.getline(answer, 5);
                        if(isValidAnswer(answer)) break;
                        else
                        {
                            cout<<"\nInvalid input. Try again\n\n";
                        }
                    }
                    send(client_socket, &answer, sizeof(answer), 0);
                }
                else if(code == END_EXAM_CODE)
                {
                    cout << "\nThanks for taking the exam\n\n";
                    int marksObtained;
                    recv(client_socket, &marksObtained, sizeof(marksObtained), 0);
                    cout << "Marks obtained: " << marksObtained << endl;
                    break;
                }
                else if(code == EMPTY_QUESTIONBANK_CODE)
                {
                    cout<<" \nTeacher did not add any questions. Try later\n\n";
                    break;
                }
            }
            break;
        }
        case 2:
        {
            int code = LEADERBOARD_CODE;
            send(client_socket, &code, sizeof(code), 0);
            char dept[10];
            strcpy(dept, this->department.c_str());
            send(client_socket, &dept, sizeof(dept), 0);
            while (1)
            {
                recv(client_socket, &code, sizeof(code), 0);
                if (code == LEADERBOARD_CODE)
                {
                    leaderboardInfo *leaderboard = new leaderboardInfo;
                    recv(client_socket, leaderboard, sizeof(*leaderboard), 0);
                    cout << "Roll: " << leaderboard->id << " Marks: " << leaderboard->marks << endl;
                }
                else if(code == END_OF_LEADERBOARD_CODE)
                {
                    cout << "\nEnd of Leaderboard\n\n";
                    break;
                }
                else
                {
                    cout<<"\nCould not get leaderboard details. Try again later.\n\n";
                    break;
                }
            }
            break;
        }
        case 3:
        {
            code = END_CONNECTION_CODE;
            send(client_socket, &code, sizeof(code), 0);
            endflag = true;
            break;
        }
        }
        if (endflag)
            break;
    }
    cout << "Connection ended :(" << endl;
    close(client_socket);
    exit(0);
}

void Teacher::user_specific_functions(int client_socket)
{
    while (1)
    {
        bool endmenu = false;
        int code;
        int ch;
        cout << "========================================\n";
        cout << " 1) Set Exam Questions\n";
        cout << " 2) See Leaderboard\n";
        cout << " 3) See Questions\n";
        cout << " 4) See Topic Leaderboard\n";
        cout << " 5) Exit\n";
        cout << "========================================\n";
        cin >> ch;
        switch (ch)
        {
        case 1:
        {
            code = SET_QUESTION_CODE;
            send(client_socket, &code, sizeof(code), 0);
            sleep(1);
            char dept[10];
            strcpy(dept, this->department.c_str());
            send(client_socket, &dept, sizeof(dept), 0); // to open specific department question bank
            while (1)
            {
                int choice;
                bool endflag = false;
                cout << "Do you want to add Question?\n 1) Yes\n 2) No\n";
                cin >> choice;
                if (choice == 1)
                {
                    code = send(client_socket, &code, sizeof(code), 0);
                    QuestionInfo *question = new QuestionInfo;
                    cin.ignore();
                    cout << set_question_menu;
                    cin.getline(question->que, 2048);
                    cin.getline(question->opt1,100);
                    cin.getline(question->opt2,100);
                    cin.getline(question->opt3,100);
                    cin.getline(question->opt4,100);
                    while(1)
                    {
                        cin.getline(question->answer,5);
                        if(isValidAnswer(question->answer))break;
                        else
                        {
                            cout<<"\ninvalid answer. Enter again\n";
                        }

                    }
                    cin.getline(question->marks,10);
                    cin.getline(question->tags,100);
                    send(client_socket, question, sizeof(*question), 0);
                }
                else
                {
                    code = END_OF_QUESTION_SETTING;
                    send(client_socket, &code, sizeof(code), 0);
                    endflag = true;
                }
                if (endflag)
                    break;
            }
            break;
        }
        case 2:
        {
            int code = LEADERBOARD_CODE;
            send(client_socket, &code, sizeof(code), 0);
            char dept[10];
            cout << "\nEnter which department leaderboard you want to see (department in abbreviated form)\n";
            cin >> dept;
            send(client_socket, &dept, sizeof(dept), 0);
            while (1)
            {
                recv(client_socket, &code, sizeof(code), 0);
                if (code == LEADERBOARD_CODE)
                {
                    leaderboardInfo *leaderboard = new leaderboardInfo;
                    recv(client_socket, leaderboard, sizeof(*leaderboard), 0);
                    cout << "Roll: " << leaderboard->id << " Marks: " << leaderboard->marks << endl;
                }
                else
                {
                    cout << "\n-----------End of Leaderboard------------\n\n";
                    break;
                }
            }
            break;
        }
        case 3:
        {
            int code = SEE_QUESTION_CODE;
            send(client_socket, &code, sizeof(code), 0);
            char dept[10];
            cout << "Enter which department questions you want to see (department in abbreviated form)\n";
            cin >> dept;
            send(client_socket, &dept, sizeof(dept), 0);
            int i=1;
            while (1)
            {
                recv(client_socket, &code, sizeof(code), 0);
                if (code == SEE_QUESTION_CODE)
                {
                    QuestionInfo *question = new QuestionInfo;
                    recv(client_socket, question, sizeof(*question), 0);
                    cout << "\nQ"<<i<<") "<<question->que << endl;
                    cout << "a: " << question->opt1 << endl;
                    cout << "b: " << question->opt2 << endl;
                    cout << "c: " << question->opt3 << endl;
                    cout << "d: " << question->opt4 << endl;
                    cout << "Correct Option: " << question->answer << endl;
                    cout << "Marks: " << question->marks << endl;
                    cout << "Topic : "<< question->tags << endl; 
                    i++;
                }
                else if(code == END_QUESTION_SEEING_CODE)
                {
                    cout << "\n---------End of questions------------\n\n";
                    break;
                }
                else
                {
                    cout<<"\n Teacher did not add any questions yet.\n\n";
                    break;
                }
            }
            break;
        }
        case 4:
        {
            int code = TOPIC_LEADERBOARD_CODE;
            send(client_socket, &code, sizeof(code), 0);
            char dept[10];
            cout << "\nEnter which department leaderboard you want to see (department in abbreviated form)\n";
            cin >> dept;
            send(client_socket, &dept, sizeof(dept), 0);
            
            // Map to store student data: student_id -> StudentPerformance
            map<string, StudentPerformance> studentData;
            
            while (1) {
                recv(client_socket, &code, sizeof(code), 0);
                if (code == TOPIC_LEADERBOARD_CODE) {
                    topicleaderboardInfo *leaderboard = new topicleaderboardInfo;
                    recv(client_socket, leaderboard, sizeof(*leaderboard), 0);
                    
                    // Convert accuracy string to double
                    double accuracy = stod(leaderboard->count);
                    
                    // Store data in the map
                    studentData[leaderboard->id].id = leaderboard->id;
                    studentData[leaderboard->id].topicAccuracies[leaderboard->topic_name] = accuracy;
                    
                    delete leaderboard;
                }
                else if (code == END_TOPIC_LEADERBOARD_CODE || code == SERVER_ERROR_CODE) {
                    break;
                }
            }
            
            // Display the performance chart
            if (!studentData.empty()) {
                displayPerformanceChart(studentData);
            } else {
                cout << "\nNo performance data available.\n";
            }
            break;
        }
        case 5:
        {
            endmenu = true;
            break;
        }
        }
        if (endmenu)
        {
            code = END_CONNECTION_CODE;
            send(client_socket, &code, sizeof(code), 0);
            break;
        }
    }
    close(client_socket);
    exit(1);
}

string parseDepartment(string id)
{
    vector<string> department;
    department.push_back("CS");
    department.push_back("EE");
    department.push_back("ECE");
    department.push_back("ME");

    for (auto it : department)
    {
        size_t index = id.find(it);
        if (index != string::npos)
            return it;
    }
    return "";
}