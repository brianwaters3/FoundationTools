/*
* Copyright (c) 2017 Sprint
* Copyright (c) 2019 Sprint
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __EFDJSON_H
#define __EFDJSON_H

#include <freeDiameter/freeDiameter-host.h>
#include "freeDiameter/libfdproto.h"
#include "freeDiameter/libfdcore.h"

/// @file
/// @brief Functions to encode/decode freeDiameter messages and grouped AVP's to and from a JSON string.

#ifdef __cplusplus
/// @brief Creates a JSON string representing the AVP values.
/// @param ref the freeDiameter message or AVP to convert to JSON.
/// @param json the destination for the JSON string.
/// @param errfunc a function that is called in the event of an error.
void fdJsonGetJSON( msg_or_avp *ref, std::string &json, void (*errfunc)(const char *) );
/// @brief Retrieves a value from a JSON string.
/// @param json the JSON string to search.
/// @param member the JSON member to search for.
/// @param value the value.
/// @return true if the member was found, otherwise false.
bool fdJsonGetValueOfMember( std::string json, std::string member, std::string &value );
/// @brief Retrieves the APN from a JSON string representing the Subscription-Data AVP.
/// @param json the JSON string to search.
/// @param apn where the APN will be stored if found.
/// @return true if the APN was found, otherwise false.
bool fdJsonGetApnValueFromSubData( std::string json, std::string &apn );
extern "C" {
#endif

/// @brief Adds the AVP from the JSON string to a freeDiameter message or grouped AVP.
/// @param json the JSON string to process.
/// @param msg the freeDiameter or grouped AVP to add to.
/// @param errfunc a function that is called in the event of an error.
/// @return 0 indicates success, otherwise failure.
int fdJsonAddAvps( const char *json, msg_or_avp *msg, void (*errfunc)(const char *) );
/// @brief Converts the AVP's from a freeDiameter message or grouped AVP to a JSON string.
/// @param msg the freeDiameter message or grouped AVP to process.
/// @param errfunc a function that is called in the event of an error.
/// @return the resulting JSON string.
const char *fdJsonGetJSON( msg_or_avp *msg, void (*errfunc)(const char *) );

/// @cond DOXYGEN_EXCLUDE
#define FDJSON_SUCCESS             0
#define FDJSON_JSON_PARSING_ERROR  1
#define FDJSON_EXCEPTION           2
/// @endcond

#ifdef __cplusplus
};
#endif

#endif /* #define __EFDJSON_H */
