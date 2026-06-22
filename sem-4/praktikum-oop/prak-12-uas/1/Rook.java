import java.util.*;
public class Rook extends Bidak {
    public Rook(int x, int y, int id) { 
        super(x,y,'R',id);
        // this.simbol = 'R';
        actions.add("1. Atas");
        actions.add("3. Kanan");
        actions.add("5. Kiri");
        actions.add("7. Bawah");
    }
    public Rook(Rook b) {
        this.x = b.x;
        this.y = b.y;
        this.simbol = b.simbol;
        this.id = b.id;
        this.actions = b.actions;
    }

    // atas
    // this.y -= steps.orElse(1);
    @Override
    public void atas(Optional<Integer> steps) {
        this.y -= steps.orElse(1);
    };

    // kanan
    // this.x += steps.orElse(1);
    @Override
    public void kanan(Optional<Integer> steps) {
        this.x += steps.orElse(1);
    };

    // kiri
    // this.x -= steps.orElse(1);
    @Override
    public void kiri(Optional<Integer> steps) {
        this.x -= steps.orElse(1);
    };

    // bawah
    // this.y += steps.orElse(1);
    @Override
    public void bawah(Optional<Integer> steps) {
        this.y += steps.orElse(1);
    };
}