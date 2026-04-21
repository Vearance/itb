public class Character {
    private String name;
    private int level;
    private double powerBase;

    public Character(String name, int level, double powerBase) {
        this.name = name;
        this.level = level;
        this.powerBase = powerBase;
    }

    public String getName() {
        return name;
    }

    public int getLevel() {
        return level;
    }

    public double getPowerBase() {
        return powerBase;
    }

    public double calculatePower() {
        return powerBase;
    }

    public void printInfo() {
        System.out.println("Nama: " + name);
        System.out.println("Nama Kapital: " + name.toUpperCase());
        System.out.println("Inisial: " + name.charAt(0));
        System.out.println("Panjang Nama: " + name.length());
        System.out.println("Level: " + level);
        System.out.println("Power Akhir: " + calculatePower());
    }
}
