package com.kuzudb;

/**
* Kuzu data type ID.
*/
public enum KuzuDataTypeID {
    ANY(0),
    NODE(10),
    REL(11),
    RECURSIVE_REL(12),
    SERIAL(13),
    BOOL(22),
    INT64(23),
    INT32(24),
    INT16(25),
    DOUBLE(27),
    FLOAT(28),
    DATE(29),
    TIMESTAMP(30),
    INTERVAL(31),
    FIXED_LIST(32),
    INTERNAL_ID(40),
    ARROW_COLUMN(41),
    STRING(50),
    BLOB(51),
    VAR_LIST(52),
    STRUCT(53),
    MAP(54),
    UNION(55);

    public final int value;

    private KuzuDataTypeID(int v) {
        this.value = v;
    }
}
