public class SummonedMonster implements ISummoned {
    private Monster monster;
    private boolean isFaceUp;
    private boolean isAttacking;
    
    public SummonedMonster(Monster m, boolean faceUp, boolean attackPos) {
        this.monster = m;
        this.isFaceUp = faceUp;
        this.isAttacking = attackPos;
    }

    @Override
    public void render() {
        String msg = "Monster ";
        msg += this.monster.getName();
        msg += " dalam keadaan ";
        if (this.isFaceUp) {
            msg += "terbuka";
        } else {
            msg += "tertutup";
        }
        msg += " dengan posisi ";
        if (this.isAttacking) {
            msg += "menyerang";
        } else {
            msg += "bertahan";
        }
        System.out.println(msg);
    }

    @Override
    public boolean flip() {
        if (!isFaceUp) {
            isFaceUp = true;
            return true;
        }
        return false;
    }
    
    @Override
    public void rotate() {
        if(isAttacking) {
            isAttacking = false;
        }
        else {
            isAttacking = true;
        }
    }
    
    @Override
    public int getPositionValue() {
        if (isAttacking) return this.monster.getAttackValue();
        else return this.monster.getDefenseValue();
    }
}
