import java.util.ArrayList;
import java.util.List;

public class Academy {
    private final List<Scroll> scrolls;

    public Academy() {
        this.scrolls = new ArrayList<>();
    }

    public void addScroll(Scroll scroll) {
        scrolls.add(scroll);
    }

    public Scroll getScroll(int index) {
        return scrolls.get(index - 1);
    }

    public int search(String keyword) {
        String loweredKeyword = keyword.toLowerCase();
        int found = 0;

        for (Scroll scroll : scrolls) {
            if (scroll.getContent().toLowerCase().contains(loweredKeyword)) {
                found++;
            }
        }

        return found;
    }

    public int count() {
        return scrolls.size();
    }
}
