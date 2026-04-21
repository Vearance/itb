public class Human implements CanRun, CanSwim {
    @Override
    public void run() {
        System.out.println("Manusia berlari.");
    }

    @Override
    public void swim() {
        System.out.println("Manusia berenang.");
    }
}
