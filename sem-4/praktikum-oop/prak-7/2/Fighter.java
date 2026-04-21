public class Fighter extends Character {
    private int physicalAtk;

    public Fighter(String name, int level, double powerBase, int physicalAtk) {
        super(name, level, powerBase);
        this.physicalAtk = physicalAtk;
    }

    public int getPhysicalAtk() {
        return physicalAtk;
    }

    @Override
    public double calculatePower() {
        return getPowerBase() + (physicalAtk * 0.7);
    }

    @Override
    public void printInfo() {
        super.printInfo();
        System.out.println("Jenis: Fighter");
    }
}
