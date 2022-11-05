//
// Created by dcavalei on 28-07-2022.
//

#ifndef INC_42_WEBSERV_MACROS_H
# define INC_42_WEBSERV_MACROS_H

# define LOG(msg) std::cout << msg << std::endl

// Debug macros
# define DEBUG // TODO: Ask Eduardo to implement this in his makefile. something like make all -DDEBUG
# ifdef DEBUG
#  define LOG_DEBUG(args) std::cout << "DEBUG " << __FILE__ << ':' << __LINE__ << " : '" << args << '\'' << std::endl
#  define LOG_ERROR(args) std::cerr << "ERROR " << __FILE__ << ':' << __LINE__ << " : '" << args << '\'' << std::endl
#  define LOG_WARNING(args) std::cout << "WARNING " << __FILE__ << ':' << __LINE__ << " : '" << args << '\'' << std::endl
# else
#  define LOG_DEBUG(args)
#  define LOG_ERROR(args)
#  define LOG_WARNING(args)
# endif

#define ROOT_STATUS_PAGE "./pages"
#define ROOT_WWW_DIR "./www"
#define STATUS_PAGE(x) std::string(ROOT_STATUS_PAGE) += x
#define WWW_PAGE(x) std::string(ROOT_WWW_DIR) += x

// Buffer semantics
# define __1KB 1024
# define __1MB __1KB * __1KB
# define __BUFFER_SIZE (__1KB * 4)
# define SUCCESS 0
# define FAILURE -1
# define DATA_ROOT "./data/"

#endif //INC_42_WEBSERV_MACROS_H
