import java.util.Optional;

public class Bishop extends Bidak {
    public Bishop(int x, int y, int id) { 
        super(x,y,'B',id);
        // this.simbol = 'B';
        actions.add("2. Atas Kanan");
        actions.add("4. Atas Kiri");
        actions.add("6. Bawah Kanan");
        actions.add("8. Bawah Kiri");
    };
    public Bishop(Bishop b) {
        this.x = b.x;
        this.y = b.y;
        this.simbol = b.simbol;
        this.id = b.id;
        this.actions = b.actions;
    }

    // atasKanan
    // this.x += steps.orElse(1); this.y -= steps.orElse(1);
    @Override
    public void atasKanan(Optional<Integer> steps) {
        this.x += steps.orElse(1); this.y -= steps.orElse(1);
    }; 

    // atasKiri
    // this.x -= steps.orElse(1); this.y -= steps.orElse(1);
    @Override
    public void atasKiri(Optional<Integer> steps) {
        this.x -= steps.orElse(1); this.y -= steps.orElse(1);
    };

    // bawahKanan
    // this.x += steps.orElse(1); this.y += steps.orElse(1); 
    @Override
    public void bawahKanan(Optional<Integer> steps) {
        this.x += steps.orElse(1); this.y += steps.orElse(1); 
    };

    // bawahKiri
    // this.x -= steps.orElse(1); this.y += steps.orElse(1); 
    @Override
    public void bawahKiri(Optional<Integer> steps) {
        this.x -= steps.orElse(1); this.y += steps.orElse(1); 
    };
}