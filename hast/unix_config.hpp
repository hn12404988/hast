#ifndef hast_unix_config_hpp
#define hast_unix_config_hpp

#include<sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/un.h>
#include <vector>
#include<unistd.h>    //close
#include <sys/epoll.h>

/******************** Error Flag *****************************
 * 1: Server doesn't exist, or socket has problem.
 * 2: Fail on sending message.
 * 3: Server's execution crash.
 * 4: No reply.
 * 5: 'forget_msg_list' or 'priority_msg_list' is blocked.
 * 6: Fail on epoll.
 * 7: Invalid message format.
 * 8: Fail on data.fd in epoll
 * 9: Fail on event in epoll
 * 10: epoll return more than 1
 *************************************************************/

//#include<arpa/inet.h> //inet_addr
//#include <netinet/in.h>

#endif /* hast_unix_config_hpp */
