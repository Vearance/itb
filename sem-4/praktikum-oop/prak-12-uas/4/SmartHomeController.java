import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Comparator;

public class SmartHomeController {

    private SmartHomeController() {}

    public static void printStatus(Object device) {
        Class<?> clazz = device.getClass();
        for (Field field : clazz.getDeclaredFields()) {
            field.setAccessible(true);
            try {
                System.out.println(field.getName() + " = " + field.get(device));
            } catch (IllegalAccessException e) {
                System.out.println("FIELD_ACCESS_ERROR");
            }
        }
    }

    public static void printCommands(Object device) {
        Class<?> clazz = device.getClass();
        Method[] methods = clazz.getDeclaredMethods();
        Arrays.sort(methods, Comparator.comparing(Method::getName));
        for (Method method : methods) {
            System.out.println(method.getName());
        }
    }

    public static void execute(Object device, String command) {
        try {
            Method method = device.getClass().getDeclaredMethod(command);
            method.setAccessible(true);
            method.invoke(device);
        } catch (NoSuchMethodException e) {
            System.out.println("COMMAND_NOT_FOUND");
        } catch (Exception e) {
            System.out.println("COMMAND_EXECUTION_ERROR");
        }
    }
}
