import java.util.Optional;
public class Knight extends Bidak {
    public Knight(int x, int y, int id) { 
        super(x,y,'K',id);
        // this.simbol = 'K';
        actions.add("2. Atas Kanan");
        actions.add("4. Atas Kiri");
        actions.add("6. Bawah Kanan");
        actions.add("8. Bawah Kiri");
    }

    public Knight(Knight b) {
        this.x = b.x;
        this.y = b.y;
        this.simbol = b.simbol;
        this.id = b.id;
        this.actions = b.actions;
    }

    // atasKanan
    // this.x += 1; this.y -= 2;
    @Override
    public void atasKanan(Optional<Integer> steps) {
        this.x += 1;
        this.y -= 2;
    };

    // atasKiri
    // this.x -= 1; this.y -= 2;
    @Override
    public void atasKiri(Optional<Integer> steps) {
        this.x -= 1; this.y -= 2;
    };

    // bawahKanan
    // this.x += 1; this.y += 2;
    @Override
    public void bawahKanan(Optional<Integer> steps) {
        this.x += 1; this.y += 2;
    };

    // bawahKiri
    // this.x -= 1; this.y += 2;
    @Override
    public void bawahKiri(Optional<Integer> steps) {
        this.x -= 1; this.y += 2;
    };
}