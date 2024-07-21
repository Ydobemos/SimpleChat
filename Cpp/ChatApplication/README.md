# SimpleChat
A simple Chat Application with a Chat-Server and Chat-Client in C++. <br>  
It uses Boost.Asio: https://www.boost.org/
I am using the (at this time of writing) current version 1.85.0. Other Versions should also work.<br>  

## Windows 
Add Include Directories in Visual Studio: <br> 
Go to `Configuration Properties` -> `C/C++` -> `General`. <br>
Find: `Additional Include Directories` and add the path to the boost header files <br> 
(For example: `C:\myFiles\Libraries\boost library\boost_1_85_0` )<br>

Remember:<br>
Visual Studio projects can have dependencies on certain DLLs, especially if you use Boost dynamic libraries.<br>
Make sure that all required DLLs are in the same directory as the .exe file.<br>
<br>
Alternatively, you can statically link the dependencies to get a standalone executable. <br>
This is possible in the project properties under `Configuration Properties` -> `C/C++` -> `Code Generation` -> `Runtime Library` by choosing /MT instead of /MD.

## Linux
Install the necessary packages if they are not already installed: <br>
`sudo apt update` <br>
`sudo apt install g++ libboost-all-dev` <br>

Compile the Server: <br>
`g++ -o chat_server chat_server.cpp -lboost_system` <br>
Compile the Client in the same way: <br>
`g++ -o chat_client chat_client.cpp -lboost_system` <br> 

## How to use 
Start the Chat-Server with a Port, for example with Port 8181, in your console/terminal: <br>
`./chat_server 8181` <br>
You might adjust the filename. So in Windows it rather looks like:<br>
`.\chat_server_cpp.exe 8181`
Now you can connect with a Client to this Server. You need the Server address and the Server port. <br>
For a localhost Server, with the Port we already used in the previous example, it looks like: <br>
`./chat_client localhost 8181` <br>
In Windows it looks like: `.\chat_client_cpp.exe localhost 8181`<br>
Open another Client in the same way in a different console/terminal to start chatting between those two Clients on the Terminal. <br>
 <br> 