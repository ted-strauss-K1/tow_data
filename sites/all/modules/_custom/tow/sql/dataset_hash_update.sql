-- CALL dataset_hash_update(1)



-- DROP PROCEDURE IF EXISTS dataset_hash_update



CREATE PROCEDURE dataset_hash_update (_nid INTEGER)

UPDATE x LEFT JOIN (
SELECT _nid as nid, CONCAT_WS(' ', t1.table_descriptions, t2.field_descriptions, t3.table_names, 
f0.int_fields, f1.char_fields, f2.float_fields, f3.bool_fields, f4.text_fields, f5.enum_fields, f6.date_fields, f7.datetime_fields, f8.time_fields, f9.timestamp_fields, f10.code_fields) AS hash
FROM
	(SELECT SQL_NO_CACHE GROUP_CONCAT( nr.body SEPARATOR ' ' ) AS table_descriptions
	FROM node_revisions nr
	INNER JOIN content_type_table t ON nr.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) t1
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT( i.description SEPARATOR ' ' ) AS field_descriptions
	FROM tow_field_info i
	INNER JOIN content_type_table t ON i.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) t2
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(n.title SEPARATOR ' ') AS table_names
	FROM node n
	INNER JOIN content_type_table t ON n.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) t3
	
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_int_value SEPARATOR ' ') AS int_fields
	FROM content_field_title_int f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f0
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_char_value SEPARATOR ' ') AS char_fields
	FROM content_field_title_char f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f1
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_float_value SEPARATOR ' ') AS float_fields
	FROM content_field_title_float f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f2
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_bool_value SEPARATOR ' ') AS bool_fields
	FROM content_field_title_bool f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f3
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_text_value SEPARATOR ' ') AS text_fields
	FROM content_field_title_text f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f4
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_enum_value SEPARATOR ' ') AS enum_fields
	FROM content_field_title_enum f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f5
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_date_value SEPARATOR ' ') AS date_fields
	FROM content_field_title_date f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f6
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_datetime_value SEPARATOR ' ') AS datetime_fields
	FROM content_field_title_datetime f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f7
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_time_value SEPARATOR ' ') AS time_fields
	FROM content_field_title_time f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f8
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_timestamp_value SEPARATOR ' ') AS timestamp_fields
	FROM content_field_title_timestamp f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f9
INNER JOIN
	(SELECT SQL_NO_CACHE GROUP_CONCAT(field_title_code_value SEPARATOR ' ') AS code_fields
	FROM content_field_title_code f
	INNER JOIN content_type_table t ON f.nid = t.nid
	WHERE t.field_dataset_value = _nid
	) f10
) t
ON x.nid = t.nid
SET x.hash = t.hash
WHERE x.nid = _nid

