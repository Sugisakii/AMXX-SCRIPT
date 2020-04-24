#if defined _sql_helper_included
	#endinput
#endif
#define _sql_helper_included

#include <amxmodx>
#include <sqlx>

const SQL_QUERY_MODE:SQL_INSERT					= any:0;
const SQL_QUERY_MODE:SQL_UPDATE					= any:1;
const SQL_QUERY_MODE:SQL_INSERT_ON_DUPLICATE	= any:2;


const __SQL_MAX_COLUMN_BUF_SIZE = 256
const __SQL_MAX_QUERY_BUF_SIZE = 1280
const __SQL_MAX_IDENTIFIER_NUMS = 4
const __SQL_TABLE_SIZE_BUFF = 32

stock Trie:_QUERY_SENTENCER
stock any:_QUERY_IDENTIFIER = 0;


enum _:__QS_ARRAY
{
	SQL_QUERY_MODE:__SQL_MODE,
	__TABLE[__SQL_TABLE_SIZE_BUFF],
	Array:__COLUMN,
	Array:__DATA,
	Array:__WHERE_C,
	Array:__WHERE_D,
	__WHERE_MANUAL[__SQL_MAX_COLUMN_BUF_SIZE]
}
/**
 * Inicia Preparacion de la oracion sql
 *SQL_INSERT_ON_DUPLICATEs
 *
 */
stock SQL_DATA:sql_init(SQL_QUERY_MODE:mode)
{
	if(!(SQL_INSERT <= mode <= SQL_INSERT_ON_DUPLICATE))
	{
		log_error(AMX_ERR_NATIVE, "[AMXX] SQL Mode not valid!")
		return any:0;
	}
	new a[__QS_ARRAY], num[__SQL_MAX_IDENTIFIER_NUMS];
	if(!_QUERY_SENTENCER)
	{
		_QUERY_SENTENCER = TrieCreate()
	}
	a[__SQL_MODE] = mode
	_QUERY_IDENTIFIER += 1;
	num_to_str(_QUERY_IDENTIFIER, num, charsmax(num))
	TrieSetArray(_QUERY_SENTENCER, num, a, sizeof(a))
	return _QUERY_IDENTIFIER;
}
/**
 * Establece la tabla a trabajar
 *
 * @param id        Identificador
 * @param table     Tabla a trabajar
 * 
 * @noreturn
 *
 */
stock sql_set_table(SQL_DATA:id, const table[])
{
	new a[__QS_ARRAY], num[__SQL_MAX_IDENTIFIER_NUMS];
	num_to_str(any:id, num, charsmax(num))
	if(TrieGetArray(_QUERY_SENTENCER, num, a, sizeof(a)) == false)
	{
		log_error(AMX_ERR_NATIVE, "[AMXX] SQL DATA ID not valid!")
		return;
	}
	formatex(a[__TABLE], charsmax(a[__TABLE]), "%s", table);
	TrieSetArray(_QUERY_SENTENCER, num, a, sizeof(a))
}
/**
 * Establece la tabla a trabajar
 *
 * @param id            Identificador
 * @param column        Establece una columna
 * @param data          Datos a establecer en una columna
 * @param ...           Datos para formatear el texto establecido en data
 *
 * @noreturn
 *
 */
stock sql_set_value(SQL_DATA:id, const column[], const data[], any:...)
{
	new a[__QS_ARRAY], num[__SQL_MAX_IDENTIFIER_NUMS], buff[__SQL_MAX_COLUMN_BUF_SIZE];
	num_to_str(any:id, num, charsmax(num))
	if(TrieGetArray(_QUERY_SENTENCER, num, a, sizeof(a)) == false)
	{
		log_error(AMX_ERR_NATIVE, "[AMXX] SQL DATA ID not valid!")
		return;
	}
	vformat(buff, charsmax(buff), data, 4);
	new Array:c = a[__COLUMN];
	new Array:d = a[__DATA];
	if(!c)
	{
		c = ArrayCreate(__SQL_MAX_COLUMN_BUF_SIZE)
		d = ArrayCreate(__SQL_MAX_COLUMN_BUF_SIZE)
		a[__COLUMN] = c
		a[__DATA] = d
	}
	replace_all(buff, charsmax(buff), "\", "\\");
	replace_all(buff, charsmax(buff), "^"", "\^"");
	
	ArrayPushString(c, column)
	ArrayPushString(d, buff)
	TrieSetArray(_QUERY_SENTENCER, num, a, sizeof(a))
}
/**
 * Actualiza datos de una columna
 * Esta funcion esta pensada para usar numeros enteros/flotantes
 * Pero se pueden usar strings
 *
 * @param id                            Identificador
 * @param column                        Establece una columna
 * @param data                          Datos a establecer en una columna
 * @param update_with_other_column      con true para actualizar con datos de otra columna
 * @param update_from                   si "update_with_other_column" se estable en true se debe establecer la columna para tomar los datos de esa columna
 * @param update_operator               se utiliza un operador logico para relizar operaciones matematicas usando de base los datos de la columna establecida por "update_from"
 *
 * @noreturn
 *
 */
stock sql_update_value(SQL_DATA:id, const column[], const data[], bool:update_with_other_column=false, const update_from[]="", const update_operator[]="")
{
	new a[__QS_ARRAY], num[__SQL_MAX_IDENTIFIER_NUMS], buff[__SQL_MAX_COLUMN_BUF_SIZE];
	num_to_str(any:id, num, charsmax(num))
	if(TrieGetArray(_QUERY_SENTENCER, num, a, sizeof(a)) == false)
	{
		log_error(AMX_ERR_NATIVE, "[AMXX] SQL DATA ID not valid!")
		return;
	}
	if(a[__SQL_MODE] != SQL_UPDATE)
	{
		log_error(AMX_ERR_NATIVE, "[AMXX] SQL DATA ID IS NOT UPDATE MODE!")
		return;
	}
	new Array:c = a[__COLUMN];
	new Array:d = a[__DATA];
	if(!c)
	{
		c = ArrayCreate(__SQL_MAX_COLUMN_BUF_SIZE)
		d = ArrayCreate(__SQL_MAX_COLUMN_BUF_SIZE)
		a[__COLUMN] = c
		a[__DATA] = d
	}
	if(update_with_other_column && update_from[0] && update_operator[0])
	{
		formatex(buff, charsmax(buff), "`%s`%s%i", update_from, update_operator, str_to_num(data));
	}
	else
	{
		copy(buff, charsmax(buff), data)
		replace_all(buff, charsmax(buff), "\", "\\");
		replace_all(buff, charsmax(buff), "^"", "\^"");
		
	}
	ArrayPushString(c, column)
	ArrayPushString(d, buff)
	TrieSetArray(_QUERY_SENTENCER, num, a, sizeof(a))
}
/**
 * Establece una condicion manual
 *
 * @param id            Identificador
 * @param where         Establece una condicion de manera manual
 * @param ...           Datos para formatear el texto establecido en where
 *
 * @noreturn
 *
 */
stock sql_set_where_manual(SQL_DATA:id, const where[], any:...)
{
	new a[__QS_ARRAY], num[__SQL_MAX_IDENTIFIER_NUMS]
	num_to_str(any:id, num, charsmax(num))
	if(TrieGetArray(_QUERY_SENTENCER, num, a, sizeof(a)) == false)
	{
		log_error(AMX_ERR_NATIVE, "[AMXX] SQL DATA ID not valid!")
		return;
	}
	vformat(a[__WHERE_MANUAL], charsmax(a[__WHERE_MANUAL]), where, 3);
	TrieSetArray(_QUERY_SENTENCER, num, a, sizeof(a))
}
/**
 * Establece una condicion Formateada
 *
 * @param id            Identificador
 * @param ...           Se establece una condicion sql preformateada con datos, 
 *                      donde el primer argumento sera la columna y el segundo 
 *                      argumento seria el dato de la columna a verificar
 *                      Este argumento tendra un "escape string", los 
 *                      argumentos siguentes argumetos se cumpliran de la misma 
 *                      manera que el argumento 1 y 2
 *
 * @note                Ejemplo: sql_set_where(id, "user", "SU^"GI")
 *                      en la oracion sql se rellenaria asi WHERE `user`="SU\"GI"
 *                      Otro Ejemplo sql_set_where(id, "user", "SU^"GI", "password", "Sugiiiiisaiiii^"^"")
 *                      en la oracion sql se rellenaria asi WHERE `user`="SU\"GI" AND `password`="Sugiiiiisaiiii\"\""
 *
 *
 * @noreturn
 *
 */
stock sql_set_where(SQL_DATA:id, any:...)
{
	new a[__QS_ARRAY], num[__SQL_MAX_IDENTIFIER_NUMS], data[__SQL_MAX_COLUMN_BUF_SIZE]
	num_to_str(any:id, num, charsmax(num))
	if(TrieGetArray(_QUERY_SENTENCER, num, a, sizeof(a)) == false)
	{
		log_error(AMX_ERR_NATIVE, "[AMXX] SQL DATA ID not valid!")
		return;
	}
	new Array:c = a[__WHERE_C];
	new Array:d = a[__WHERE_D];
	if(!c)
	{
		c = ArrayCreate(__SQL_MAX_COLUMN_BUF_SIZE)
		d = ArrayCreate(__SQL_MAX_COLUMN_BUF_SIZE)
		a[__WHERE_C] = c
		a[__WHERE_D] = d
	}
	new pos, ch;
	for(new i = 1 ; i < numargs() ; i++)
	{
		pos = 0;
		while((ch = getarg(i, pos )))
		{
			data[pos++] = ch;
		}
		data[pos++] = '^0';
		if((i % 2) == 0)
		{
			replace_all(data, charsmax(data), "\", "\\");
			replace_all(data, charsmax(data), "^"", "\^"");
			ArrayPushString(d, data);
		}
		else
		{
			ArrayPushString(c, data);
		}
	}
	TrieSetArray(_QUERY_SENTENCER, num, a, sizeof(a))
}
/**
 * Ejecuta una una Oracion SQL 
 *
 * @note The handler should look like:
 *       public QueryHandler(failstate, Handle:query, error[], errnum, data[], size, Float:queuetime)
 *       failstate - One of the three TQUERY_ defines.
 *       query     - Handle to the query, do not free it.
 *       error     - An error message, if any.
 *       errnum    - An error code, if any.
 *       data      - Data array you passed in.
 *       size      - Size of the data array you passed in.
 *       queuetime - Amount of gametime that passed while the query was resolving.
 * @note This will not interrupt gameplay in the event of a poor/lossed 
 *       connection, however, the interface is more complicated and 
 *       asynchronous. Furthermore, a new connection/disconnection is 
 *       made for each query to simplify driver support.
 * @note The handle does not need to be freed.
 *
 * @param id            Identificador
 * @param SQL_TUPLE     Tuple handle, returned from SQL_MakeDbTuple().
 * @param Handler       A function to be called when the query finishes. It has to be public.
 * @param data          Additional data array that will be passed to the handler function.
 * @param dataSize      The size of the additional data array. 
 * 
 *
 * @noreturn
 *
 */
stock sql_execute(SQL_DATA:id, Handle:SQL_TUPLE, const Handler[], const szData[]="", DataSize=0)
{
	new a[__QS_ARRAY], num[__SQL_MAX_IDENTIFIER_NUMS], data[__SQL_MAX_COLUMN_BUF_SIZE], query[__SQL_MAX_QUERY_BUF_SIZE], len;
	num_to_str(any:id, num, charsmax(num))
	if(TrieGetArray(_QUERY_SENTENCER, num, a, sizeof(a)) == false)
	{
		log_error(AMX_ERR_NATIVE, "[AMXX] SQL DATA ID not valid!")
		return;
	}
	/*
	len = formatex(query, charsmax(query), "%s `%s` ", 
	a[__SQL_MODE] == SQL_INSERT ? "INSERT INTO" : 
	a[__SQL_MODE] == SQL_UPDATE ? "UPDATE" : "", a[__TABLE])
	*/
	switch(a[__SQL_MODE])
	{
		case SQL_INSERT, SQL_INSERT_ON_DUPLICATE:
		{
			len = copy(query, charsmax(query), "INSERT INTO");
		}
		case SQL_UPDATE:
		{
			len = copy(query, charsmax(query), "UPDATE");
		}
	}
	len += formatex(query[len], charsmax(query), " `%s` ", a[__TABLE]);

	new Array:d, Array:c, i;
	c = a[__COLUMN];
	d = a[__DATA];

	
	if(a[__SQL_MODE] == SQL_INSERT || a[__SQL_MODE] == SQL_INSERT_ON_DUPLICATE)
	{
		len += add(query[len], charsmax(query), "(");
		for( i = 0 ; i < ArraySize(c) ; i++)
		{
			ArrayGetString(c, i, data, charsmax(data))
			len += formatex(query[len], charsmax(query), "%s`%s`", i > 0 ? "," : "", data);
		}
		len += add(query[len], charsmax(query), ") VALUES (");
		for( i = 0 ; i < ArraySize(d) ; i++)
		{
			ArrayGetString(d, i, data, charsmax(data))
			len += formatex(query[len], charsmax(query), "%s^"%s^"", i > 0 ? "," : "", data);
		}
		len += add(query[len], charsmax(query), ")");
	}
	if(a[__SQL_MODE] == SQL_UPDATE || a[__SQL_MODE] == SQL_INSERT_ON_DUPLICATE)
	{
		if(a[__SQL_MODE] == SQL_INSERT_ON_DUPLICATE)
		{
			len += add(query[len], charsmax(query), " ON DUPLICATE KEY UPDATE ");
		}
		else
		{
			len += add(query[len], charsmax(query), "SET ");
		}
		new data2[__SQL_MAX_COLUMN_BUF_SIZE]
		for( i = 0 ; i < ArraySize(c) ; i++)
		{
			ArrayGetString(c, i, data, charsmax(data))
			ArrayGetString(d, i, data2, charsmax(data2))
			len += formatex(query[len], charsmax(query), "%s`%s`=%s%s%s", i > 0 ? "," : "", data, data2[0] != '`' ? "^"" : "", data2, data2[0] != '`' ? "^"" : "");
		}
		if(a[__SQL_MODE] != SQL_INSERT_ON_DUPLICATE)
		{
			if(!a[__WHERE_MANUAL])
			{
				new Array:wc, Array:wd;
				wd = a[__WHERE_D]
				wc = a[__WHERE_C]
				len += add(query[len], charsmax(query), " WHERE ");
				for(new i = 0 ; i < ArraySize(wc) ; i++)
				{
					ArrayGetString(wc, i, data, charsmax(data))
					ArrayGetString(wd, i, data2, charsmax(data2))
					len += formatex(query[len], charsmax(query), "%s`%s`=^"%s^"", i > 0 ? " AND " : "", data, data2)
				}
				ArrayDestroy(wc)
				ArrayDestroy(wd)
			}
			else
			{
				len += formatex(query[len], charsmax(query), " WHERE %s", a[__WHERE_MANUAL]);
			}
		}
	}
	
	//server_print(query)
	ArrayDestroy(c)
	ArrayDestroy(d)
	TrieDeleteKey(_QUERY_SENTENCER, num)
	SQL_ThreadQuery(SQL_TUPLE, Handler, query, szData, DataSize);
}
