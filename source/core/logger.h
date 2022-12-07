#ifndef __LOGGER_H__
#define __LOGGER_H__
 
#define C_NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"

#include "mbed.h"
#if defined LOG_LEVEL_TRACE
#define GSH_TRACE(x, ...) std::printf("[TRACE: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_DEBUG(x, ...) std::printf(BLUE "[DEBUG: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_INFO(x, ...)  std::printf(GREEN "[INFO: %s:%d]" C_NONE x" \r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_WARN(x, ...)  std::printf(YELLOW "[WARN: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_ERROR(x, ...) std::printf(RED "[ERROR: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
 
#elif defined LOG_LEVEL_DEBUG
#define GSH_TRACE(x, ...)
#define GSH_DEBUG(x, ...) std::printf(BLUE "[DEBUG: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_INFO(x, ...)  std::printf(GREEN "[INFO: %s:%d]" C_NONE x" \r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_WARN(x, ...)  std::printf(YELLOW "[WARN: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_ERROR(x, ...) std::printf(RED "[ERROR: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
 
#elif defined LOG_LEVEL_INFO
#define GSH_TRACE(x, ...)
#define GSH_DEBUG(x, ...) 
#define GSH_INFO(x, ...)  std::printf(GREEN "[INFO: %s:%d]" C_NONE x" \r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_WARN(x, ...)  std::printf(YELLOW "[WARN: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_ERROR(x, ...) std::printf(RED "[ERROR: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
 
#elif defined LOG_LEVEL_WARN
#define GSH_TRACE(x, ...)
#define GSH_DEBUG(x, ...)
#define GSH_INFO(x, ...)
#define GSH_WARN(x, ...)  std::printf(YELLOW "[WARN: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define GSH_ERROR(x, ...) std::printf(RED "[ERROR: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
 
#elif defined LOG_LEVEL_ERROR
#define GSH_TRACE(x, ...)
#define GSH_DEBUG(x, ...)
#define GSH_INFO(x, ...)
#define GSH_WARN(x, ...)
#define GSH_ERROR(x, ...) std::printf(RED "[ERROR: %s:%d]" C_NONE x "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
 
#else
#define GSH_TRACE(x, ...)
#define GSH_DEBUG(x, ...)
#define GSH_INFO(x, ...)
#define GSH_WARN(x, ...)
#define GSH_ERROR(x, ...)
#endif
 
#endif