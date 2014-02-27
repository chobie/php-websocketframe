/*
 * PHP Websocket Frame extension
 * Copyright 2013 Shuhei Tanuma.  All rights reserved.
 *
 * https://github.com/chobie/php-websocketframe
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "php_websocketframe.h"

#ifdef _MSC_VER
  #if defined(_M_IX86) && \
      !defined(PHP_WEBSOCKETFRAME_DISABLE_LITTLE_ENDIAN_OPT_FOR_TEST)
    #define WEBSOCKETFRAME_LITTLE_ENDIAN 1
  #endif
  #if _MSC_VER >= 1300
    // If MSVC has "/RTCc" set, it will complain about truncating casts at
    // runtime.  This file contains some intentional truncating casts.
    #pragma runtime_checks("c", off)
  #endif
#else
  #include <sys/param.h>   // __BYTE_ORDER
  #ifdef __APPLE__
    #define __BIG_ENDIAN __DARWIN_BIG_ENDIAN
    #define __LITTLE_ENDIAN __DARWIN_LITTLE_ENDIAN
    #define __BYTE_ORDER __DARWIN_BYTE_ORDER
  #endif

  #if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN && \
      !defined(PHP_WEBSOCKETFRAME_DISABLE_LITTLE_ENDIAN_OPT_FOR_TEST)
    #define WEBSOCKETFRAME_LITTLE_ENDIAN 1
  #endif
#endif

typedef struct {
	zend_object zo;
	zend_bool mutable;
	zend_bool fin;
	zend_bool rsv1;
	zend_bool rsv2;
	zend_bool rsv3;
	zend_bool mask;
	unsigned char mask_key[4];
	uint16_t opcode;
	uint64_t payload_length;
	unsigned char *payload;
} php_websocketframe;


#  if ZEND_MODULE_API_NO >= 20100525
#  define PHP_WEBSOCKETFRAME_STD_CREATE_OBJECT(STRUCT_NAME) \
	STRUCT_NAME *object;\
	\
	object = (STRUCT_NAME*)ecalloc(1, sizeof(*object));\
	zend_object_std_init(&object->zo, ce TSRMLS_CC);\
	object_properties_init(&object->zo, ce);\
	\
	retval.handle = zend_objects_store_put(object,\
		(zend_objects_store_dtor_t)zend_objects_destroy_object,\
		(zend_objects_free_object_storage_t) STRUCT_NAME##_free_storage ,\
	NULL TSRMLS_CC);\
	retval.handlers = zend_get_std_object_handlers();
#  else
#  define PHP_WEBSOCKETFRAME_STD_CREATE_OBJECT(STRUCT_NAME) \
	STRUCT_NAME *object;\
	zval *tmp = NULL;\
	\
	object = (STRUCT_NAME*)ecalloc(1, sizeof(*object));\
	zend_object_std_init(&object->zo, ce TSRMLS_CC);\
	zend_hash_copy(object->zo.properties, &ce->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *)); \
	\
	retval.handle = zend_objects_store_put(object,\
		(zend_objects_store_dtor_t)zend_objects_destroy_object,\
		(zend_objects_free_object_storage_t) STRUCT_NAME##_free_storage ,\
	NULL TSRMLS_CC);\
	retval.handlers = zend_get_std_object_handlers();
#  endif


static void php_websocketframe_free_storage(php_websocketframe *object TSRMLS_DC)
{
	if (object->payload_length > 0) {
		efree(object->payload);
	}
	zend_object_std_dtor(&object->zo TSRMLS_CC);
	efree(object);
}

zend_object_value php_websocketframe_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;

	PHP_WEBSOCKETFRAME_STD_CREATE_OBJECT(php_websocketframe);
	object->fin = 1;
	object->rsv1 = 0;
	object->rsv2 = 0;
	object->rsv3 = 0;
	object->mask = 0;
	object->opcode = 1;
	//object->mask_key = NULL;
	object->payload_length = 0;
	object->payload = NULL;
	object->mutable = 1;

	return retval;
}

zend_class_entry *php_websocketframe_class_entry;

ZEND_DECLARE_MODULE_GLOBALS(websocketframe);

ZEND_BEGIN_ARG_INFO_EX(arginfo_websocketframe_parse_from_string, 0, 0, 1)
	ZEND_ARG_INFO(0, bytes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_websocketframe_serialize_to_string, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_websocketframe_set_payload, 0, 0, 1)
	ZEND_ARG_INFO(0, bytes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_websocketframe_set_opcode, 0, 0, 1)
	ZEND_ARG_INFO(0, opcode)
ZEND_END_ARG_INFO()

/* {{{ proto WebSocketFrame WebSocketFrame::__construct()
*/
PHP_METHOD(websocketframe, __construct)
{
}
/* }}} */

/* {{{ proto void WebSocketFrame::setOpcode($opcode)
*/
PHP_METHOD(websocketframe, setOpcode)
{
	long opcode;
	php_websocketframe *frame = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"l", &opcode) == FAILURE) {
		return;
	}

	if (opcode < 0 || opcode > 0xF) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0 TSRMLS_CC, "opcode have to be 0x0 - 0x0f");
		return;
	}

	frame = (php_websocketframe*)zend_object_store_get_object(getThis() TSRMLS_CC);
	if (frame->mutable == 0) {
		zend_throw_exception_ex(spl_ce_LogicException, 0 TSRMLS_CC, "this object is not mutable. please use new one.");
		return;
	}

	frame->opcode = opcode;
}
/* }}} */


/* {{{ proto void WebSocketFrame::setPayload($payload)
*/
PHP_METHOD(websocketframe, setPayload)
{
	const char *bytes;
	int bytes_len = 0;
	php_websocketframe *frame = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s", &bytes, &bytes_len) == FAILURE) {
		return;
	}

	frame = (php_websocketframe*)zend_object_store_get_object(getThis() TSRMLS_CC);
	if (frame->mutable == 0) {
		zend_throw_exception_ex(spl_ce_LogicException, 0 TSRMLS_CC, "this object is not mutable. please use new one.");
		return;
	}

	if (frame->payload_length > 0) {
		efree(frame->payload);
		frame->payload_length = 0;
	}

    frame->payload = emalloc(sizeof(unsigned char*) * bytes_len);
    memcpy(frame->payload, bytes, bytes_len);
    frame->payload_length = bytes_len;
}
/* }}} */

/* {{{ proto string WebSocketFrame::getPayload()
*/
PHP_METHOD(websocketframe, getPayload)
{
	php_websocketframe *frame = NULL;

	frame = (php_websocketframe*)zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_STRINGL(frame->payload, frame->payload_length, 1);
}
/* }}} */

/* {{{ proto string WebSocketFrame::getOpcode()
*/
PHP_METHOD(websocketframe, getOpcode)
{
	php_websocketframe *frame = NULL;

	frame = (php_websocketframe*)zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_LONG(frame->opcode);
}
/* }}} */


/* {{{ proto string WebSocketFrame::serializeToString()
*/
PHP_METHOD(websocketframe, serializeToString)
{
	unsigned char *buffer;
	size_t frame_size = 2, offset = 2;
	php_websocketframe *frame = NULL;

	frame = (php_websocketframe*)zend_object_store_get_object(getThis() TSRMLS_CC);
	if (frame->payload_length == 0) {
		zend_throw_exception_ex(spl_ce_LogicException, 0 TSRMLS_CC, "empty payload");
		return;
	}

	if (frame->mask) {
		frame_size += 4;
	}

	if (frame->payload_length >= 0x7e && frame->payload_length <= 0x10000) {
		frame_size += 2;
	} else if (frame->payload_length >= 0x10000) {
		frame_size += 8;
	}

	frame_size += frame->payload_length;
	buffer = emalloc(sizeof(unsigned char) * frame_size + 1);
	memset(buffer, 0, frame_size + 1);

	buffer[0] = 0;
	if (frame->fin) {
		buffer[0] |= 0x80;
	}
	if (frame->rsv1) {
		buffer[0] |= 0x10;
	}
	if (frame->rsv2) {
		buffer[0] |= 0x20;
	}
	if (frame->rsv3) {
		buffer[0] |= 0x40;
	}
	// type
	buffer[0] |= frame->opcode;
	buffer[1] = 0;

	if (frame->payload_length < 0x7e) {
		buffer[1] |= frame->payload_length;
	} else if (frame->payload_length < 0x10000) {
		union {
			uint16_t length;
			uint8_t buffer[2];
		} u;

#ifdef WEBSOCKETFRAME_LITTLE_ENDIAN
		u.length = frame->payload_length;
		u.length = (u.length >> 8) | (u.length << 8);
#else
		u.length = frame->payload_length;
#endif

		buffer[1] |= 0x7e;

		memcpy(&buffer[offset], u.buffer, 2);
		offset += 2;
	} else {
		union {
			uint64_t length;
			uint8_t buffer[8];
		} u;

#ifdef WEBSOCKETFRAME_LITTLE_ENDIAN
		u.length = frame->payload_length;
		u.length = (u.length >> 56) |
				((u.length<<40) & 0x00FF000000000000) |
				((u.length<<24) & 0x0000FF0000000000) |
				((u.length<<8) & 0x000000FF00000000) |
				((u.length>>8) & 0x00000000FF000000) |
				((u.length>>24) & 0x0000000000FF0000) |
				((u.length>>40) & 0x000000000000FF00) |
				(u.length << 56);
#else
		u.length = frame->payload_length;
#endif

		buffer[1] |= 0x7f;
		memcpy(&buffer[offset], &u.length, 8);
		offset += 8;
	}

	if (frame->mask) {
		// TODO: implement MASK DATA
	}

	memcpy(&buffer[offset], frame->payload, frame->payload_length);

	Z_TYPE_P(return_value) = IS_STRING;
	Z_STRLEN_P(return_value) = frame_size;
	Z_STRVAL_P(return_value) = buffer;
}
/* }}} */

/* {{{ proto WebSocketFrame WebSocketFrame::parseFromString($bytes)
*/
PHP_METHOD(websocketframe, parseFromString)
{
	const char *bytes;
	int bytes_len = 0;
	zval *instance;
	HashTable *properties = NULL;
	php_websocketframe *frame = NULL;
	int32_t offset = 2;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s", &bytes, &bytes_len) == FAILURE) {
		return;
	}

	MAKE_STD_ZVAL(instance);
	object_init_ex(instance, php_websocketframe_class_entry);
	ALLOC_HASHTABLE(properties);
	zend_hash_init(properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_merge_properties(instance, properties, 1 TSRMLS_CC);

	frame = (php_websocketframe*)zend_object_store_get_object(instance TSRMLS_CC);
	frame->fin = ((bytes[0] & 0x80) >> 7);
	frame->opcode = (bytes[0] & 0x0f);
	frame->mask = ((bytes[1] & 0x80) >> 7);
	frame->payload_length = (bytes[1] & 0x7f);

	if (frame->payload_length == 0x7e) {
		uint16_t length;

#ifdef WEBSOCKETFRAME_LITTLE_ENDIAN
		memcpy(&length, &bytes[offset], 2);
		frame->payload_length = (length >> 8) | (length << 8);
#else
		memcpy(&length, &bytes[offset], 2);
		frame->payload_length = (uint64_t)length;
#endif

		offset += 2;
	} else if (frame->payload_length == 0x7f) {
		uint64_t length;

#ifdef WEBSOCKETFRAME_LITTLE_ENDIAN
		memcpy(&length, &bytes[offset], 8);
		frame->payload_length = (uint64_t)length;
#else
		memcpy(&length, &bytes[offset], 8);
		frame->payload_length = (length >> 56) |
									  ((length<<40) & 0x00FF000000000000) |
									  ((length<<24) & 0x0000FF0000000000) |
									  ((length<<8) & 0x000000FF00000000) |
									  ((length>>8) & 0x00000000FF000000) |
									  ((length>>24) & 0x0000000000FF0000) |
									  ((length>>40) & 0x000000000000FF00) |
									  (length << 56);
#endif

		memcpy(length, bytes[offset], 8);
		offset += 8;
	}

	memcpy(frame->mask_key, &bytes[offset], 4);
	offset += 4;
	frame->payload = ecalloc(1, sizeof(unsigned char) * frame->payload_length);
	memcpy(frame->payload, &bytes[offset], frame->payload_length);

	if (frame->mask) {
		uint64_t i;

		for (i = 0; i < frame->payload_length; i++) {
			frame->payload[i] = frame->payload[i] ^ frame->mask_key[i % 4];
		}
	}
	frame->mutable = 0;

	RETURN_ZVAL(instance, 0, 1);
}

/* }}} */

static zend_function_entry php_websocketframe_methods[] = {
	PHP_ME(websocketframe, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	PHP_ME(websocketframe, setOpcode, arginfo_websocketframe_set_opcode, ZEND_ACC_PUBLIC)
	PHP_ME(websocketframe, getOpcode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(websocketframe, getPayload, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(websocketframe, setPayload, arginfo_websocketframe_set_payload, ZEND_ACC_PUBLIC)
	PHP_ME(websocketframe, serializeToString, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(websocketframe, parseFromString, arginfo_websocketframe_parse_from_string, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static void php_websocketframe_init(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "WebsocketFrame", php_websocketframe_methods);
	php_websocketframe_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	php_websocketframe_class_entry->create_object = php_websocketframe_new;

	zend_declare_class_constant_long(php_websocketframe_class_entry, ZEND_STRS("OP_CONTENUATION")-1, 0x00 TSRMLS_CC);
	zend_declare_class_constant_long(php_websocketframe_class_entry, ZEND_STRS("OP_TEXT")-1, 0x01 TSRMLS_CC);
	zend_declare_class_constant_long(php_websocketframe_class_entry, ZEND_STRS("OP_BINARY")-1, 0x02 TSRMLS_CC);
	zend_declare_class_constant_long(php_websocketframe_class_entry, ZEND_STRS("OP_PING")-1, 0x08 TSRMLS_CC);
	zend_declare_class_constant_long(php_websocketframe_class_entry, ZEND_STRS("OP_PONG")-1, 0x09 TSRMLS_CC);

}

PHP_MINFO_FUNCTION(websocketframe)
{
	php_printf("PHP Websocket Frame Extension\n");

	php_info_print_table_start();
	php_info_print_table_header(2, "WebsocketFrame Support", "enabled");
	php_info_print_table_row(2, "Version", PHP_WEBSOCKETFRAME_VERSION);
	php_info_print_table_end();
}

PHP_INI_BEGIN()
PHP_INI_END()

static PHP_GINIT_FUNCTION(websocketframe)
{
}

static PHP_GSHUTDOWN_FUNCTION(websocketframe)
{
}

PHP_MINIT_FUNCTION(websocketframe)
{
	REGISTER_INI_ENTRIES();

	php_websocketframe_init(TSRMLS_C);
	return SUCCESS;
}

PHP_RINIT_FUNCTION(websocketframe)
{
	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(websocketframe)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(websocketframe)
{
	return SUCCESS;
}

zend_module_entry websocketframe_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"websocketframe",
	NULL,					/* Functions */
	PHP_MINIT(websocketframe),	/* MINIT */
	PHP_MSHUTDOWN(websocketframe),	/* MSHUTDOWN */
	PHP_RINIT(websocketframe),	/* RINIT */
	PHP_RSHUTDOWN(websocketframe),		/* RSHUTDOWN */
	PHP_MINFO(websocketframe),	/* MINFO */
#if ZEND_MODULE_API_NO >= 20010901
	PHP_WEBSOCKETFRAME_VERSION,
#endif
	PHP_MODULE_GLOBALS(websocketframe),
	PHP_GINIT(websocketframe),
	PHP_GSHUTDOWN(websocketframe),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_WEBSOCKETFRAME
ZEND_GET_MODULE(websocketframe)
#endif
