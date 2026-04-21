public class Mage extends Character {
    private int magicalAtk;

    public Mage(String name, int level, double powerBase, int magicalAtk) {
        super(name, level, powerBase);
        this.magicalAtk = magicalAtk;
    }

    public int getMagicalAtk() {
        return magicalAtk;
    }

    @Override
    public double calculatePower() {
        return getPowerBase() + (magicalAtk * 0.5);
    }

    @Override
    public void printInfo() {
        super.printInfo();
        System.out.println("Jenis: Mage");
    }
}
