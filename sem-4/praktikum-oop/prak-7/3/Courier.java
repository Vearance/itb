public class Courier {
    protected String name;
    protected int speed;

    public Courier() {
        this.name = "Unknown";
        this.speed = 10;
    }

    public Courier(String name, int speed) {
        this.name = name;
        this.speed = speed;
    }

    public String deliver(String item) {
        return name + " delivers " + item;
    }

    public int getSpeed() {
        return speed;
    }

    @Override
    public String toString() {
        return name + " with speed " + speed + " is ready to deliver";
    }
}