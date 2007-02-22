/*
 * Copyright 2007 Arnaud Cornet.
 *
 * TODO: chose and display license ;)
 *
 */

#include <ruby.h>
#include <dbus/dbus.h>
#include <alloca.h>

void Init_dbus_bus(void);

VALUE mDBus;
VALUE cDBusConnection;
VALUE eDBusException;
VALUE cDBusMessage;

void rubydbus_exception(DBusError *error)
{
	char *rubymessage;
	/* not leaking when using exception hurts the head */

	rubymessage = alloca(strlen(error->name) + strlen(": ") +
			strlen(error->message) + 1);
	strcpy(rubymessage, error->name);
	strcpy(rubymessage, ": ");
	strcpy(rubymessage, error->message);

	dbus_error_free(error);

	rb_raise(eDBusException, error->message);
}

static VALUE rubydbus_connection_new(VALUE class, VALUE address)
{
	DBusConnection *connection;
	DBusError error;
	VALUE rconnection;

	connection = dbus_connection_open(StringValuePtr(address), &error);
	if (connection == NULL)
		rubydbus_exception(&error);

	rconnection = Data_Wrap_Struct(cDBusConnection, 0,
			dbus_connection_unref, connection);
	rb_obj_call_init(rconnection, 0, 0);
	return rconnection;
}

static VALUE rubydbus_connection_new_private(VALUE class, VALUE address)
{
	DBusConnection *connection;
	DBusError error;
	VALUE rconnection;

	connection = dbus_connection_open_private(StringValuePtr(address),
			&error);
	if (connection == NULL)
		rubydbus_exception(&error);

	rconnection = Data_Wrap_Struct(cDBusConnection, 0,
			dbus_connection_unref, connection);
	rb_obj_call_init(rconnection, 0, 0);
	return rconnection;
}

static VALUE rubydbus_connection_pop_message(VALUE self)
{
	DBusConnection *connection;
	DBusMessage *message;
	Data_Get_Struct(self, DBusConnection, connection);

	message = dbus_connection_pop_message(connection);
	return Data_Wrap_Struct(cDBusMessage, 0, dbus_message_unref, message);
}

static VALUE rubydbus_connection_close(VALUE self)
{
	DBusConnection *connection;

	Data_Get_Struct(self, DBusConnection, connection);
	dbus_connection_close(connection);
	return self;
}

static VALUE rubydbus_message_new(VALUE class, VALUE msg_type)
{
	DBusMessage *message;
	VALUE ret;

	message = dbus_message_new(NUM2INT(msg_type));

	ret = Data_Wrap_Struct(cDBusMessage, 0,
			dbus_message_unref, message);
	return ret;
}

void Init_dbusglue(void)
{
	mDBus = rb_define_module("DBus");

	eDBusException = rb_define_class_under(mDBus, "Exception",
			rb_eException);

	cDBusConnection = rb_define_class_under(mDBus, "Connection",
			rb_cObject);
	rb_define_singleton_method(cDBusConnection, "new",
			rubydbus_connection_new, 1);
	/* maybe use a ruby alias */
	rb_define_singleton_method(cDBusConnection, "open",
			rubydbus_connection_new, 1);
	rb_define_singleton_method(cDBusConnection, "new_private",
			rubydbus_connection_new_private, 1);
	rb_define_singleton_method(cDBusConnection, "open_private",
			rubydbus_connection_new_private, 1);

	rb_define_method(cDBusConnection, "pop_message",
			rubydbus_connection_pop_message, 0);
	rb_define_method(cDBusConnection, "close",
			rubydbus_connection_close, 0);

	cDBusMessage = rb_define_class_under(mDBus, "Message",
			rb_cObject);
	rb_define_const(cDBusMessage, "TYPE_INVALID",
			INT2NUM(DBUS_MESSAGE_TYPE_INVALID));
	rb_define_const(cDBusMessage, "TYPE_METHOD_CALL",
			INT2NUM(DBUS_MESSAGE_TYPE_METHOD_CALL));
	rb_define_const(cDBusMessage, "TYPE_METHOD_RETURN",
			INT2NUM(DBUS_MESSAGE_TYPE_METHOD_RETURN));
	rb_define_const(cDBusMessage, "TYPE_ERROR",
			INT2NUM(DBUS_MESSAGE_TYPE_ERROR));
	rb_define_const(cDBusMessage, "TYPE_SIGNAL",
			INT2NUM(DBUS_MESSAGE_TYPE_SIGNAL));
	rb_define_singleton_method(cDBusConnection, "new",
			rubydbus_message_new, 1);
	Init_dbus_bus();
}
