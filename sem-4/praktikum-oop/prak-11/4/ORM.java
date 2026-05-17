import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class ORM {

	public static void schema(Class<?> clazz) {
		System.out.println("Table: " + getTableName(clazz));
		System.out.println("Columns:");

		for (Field field : clazz.getDeclaredFields()) {
			ColumnName columnName = field.getAnnotation(ColumnName.class);
			if (columnName == null) {
				continue;
			}

			String name = columnName.value().isEmpty() ? field.getName() : columnName.value();
			System.out.print("  - " + name);
			if (columnName.primaryKey()) {
				System.out.print(" [PRIMARY KEY]");
			}
			System.out.println();
		}
	}

	public static Object createInstance(Class<?> clazz, String[] values) throws Exception {
		Constructor<?> constructor = clazz.getDeclaredConstructor();
		constructor.setAccessible(true);
		Object instance = constructor.newInstance();

		Field[] fields = clazz.getDeclaredFields();
		int valueIndex = 0;
		for (Field field : fields) {
			ColumnName columnName = field.getAnnotation(ColumnName.class);
			if (columnName == null) {
				continue;
			}

			field.setAccessible(true);
			field.set(instance, convertValue(field.getType(), values[valueIndex++]));
		}

		invokeHooks(instance, Hook.When.POST_LOAD);
		return instance;
	}

	public static void insert(Object obj) {
		try {
			invokeHooks(obj, Hook.When.PRE_INSERT);
			System.out.println(buildInsertSql(obj));
		} catch (Exception e) {
			System.out.println("Gagal insert: " + getMessage(e));
		}
	}

	public static void delete(Object obj) {
		try {
			invokeHooks(obj, Hook.When.PRE_DELETE);
			System.out.println(buildDeleteSql(obj));
		} catch (Exception e) {
			System.out.println("Gagal delete: " + getMessage(e));
		}
	}

	private static String getTableName(Class<?> clazz) {
		TableName tableName = clazz.getAnnotation(TableName.class);
		return tableName == null ? clazz.getSimpleName().toLowerCase() : tableName.value();
	}

	private static void invokeHooks(Object obj, Hook.When when) throws Exception {
		for (Method method : obj.getClass().getDeclaredMethods()) {
			Hook hook = method.getAnnotation(Hook.class);
			if (hook != null && hook.when() == when) {
				method.setAccessible(true);
				method.invoke(obj);
			}
		}
	}

	private static String buildInsertSql(Object obj) throws IllegalAccessException {
		Class<?> clazz = obj.getClass();
		StringBuilder columns = new StringBuilder();
		StringBuilder values = new StringBuilder();
		boolean first = true;

		for (Field field : clazz.getDeclaredFields()) {
			ColumnName columnName = field.getAnnotation(ColumnName.class);
			if (columnName == null) {
				continue;
			}

			String column = columnName.value().isEmpty() ? field.getName() : columnName.value();
			field.setAccessible(true);
			Object value = field.get(obj);

			if (!first) {
				columns.append(", ");
				values.append(", ");
			}

			columns.append(column);
			values.append(formatSqlValue(value));
			first = false;
		}

		return "INSERT INTO " + getTableName(clazz) + " (" + columns + ") VALUES (" + values + ")";
	}

	private static String buildDeleteSql(Object obj) throws IllegalAccessException {
		Class<?> clazz = obj.getClass();
		Field primaryKeyField = null;
		ColumnName primaryKeyColumn = null;

		for (Field field : clazz.getDeclaredFields()) {
			ColumnName columnName = field.getAnnotation(ColumnName.class);
			if (columnName != null && columnName.primaryKey()) {
				primaryKeyField = field;
				primaryKeyColumn = columnName;
				break;
			}
		}

		if (primaryKeyField == null || primaryKeyColumn == null) {
			throw new IllegalStateException("Primary key not found");
		}

		primaryKeyField.setAccessible(true);
		Object value = primaryKeyField.get(obj);
		String column = primaryKeyColumn.value().isEmpty() ? primaryKeyField.getName() : primaryKeyColumn.value();

		return "DELETE FROM " + getTableName(clazz) + " WHERE " + column + " = " + formatSqlValue(value);
	}

	private static Object convertValue(Class<?> type, String value) {
		if (type == String.class) {
			return value;
		}
		if (type == int.class || type == Integer.class) {
			return Integer.parseInt(value);
		}
		if (type == long.class || type == Long.class) {
			return Long.parseLong(value);
		}
		if (type == double.class || type == Double.class) {
			return Double.parseDouble(value);
		}
		if (type == float.class || type == Float.class) {
			return Float.parseFloat(value);
		}
		if (type == boolean.class || type == Boolean.class) {
			return Boolean.parseBoolean(value);
		}
		return value;
	}

	private static String formatSqlValue(Object value) {
		if (value instanceof String) {
			return "'" + value + "'";
		}
		return String.valueOf(value);
	}

	private static String getMessage(Exception e) {
		Throwable cause = e.getCause();
		return cause != null && cause.getMessage() != null ? cause.getMessage() : e.getMessage();
	}
}
