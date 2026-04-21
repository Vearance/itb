public class CipherScroll extends Scroll {
    private final int shift;

    public CipherScroll(String content, int shift) {
        super(content);
        this.shift = shift;
    }

    @Override
    protected String getType() {
        return "CIPHER";
    }

    @Override
    public String process() {
        return caesar(getContent(), shift);
    }

    public String decode() {
        return caesar(getContent(), -shift);
    }

    private String caesar(String text, int shiftValue) {
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if (c >= 'a' && c <= 'z') {
                int base = 'a';
                char encrypted = (char) (((c - base + shiftValue % 26 + 26) % 26) + base);
                result.append(encrypted);
            } else if (c >= 'A' && c <= 'Z') {
                int base = 'A';
                char encrypted = (char) (((c - base + shiftValue % 26 + 26) % 26) + base);
                result.append(encrypted);
            } else {
                result.append(c);
            }
        }
        return result.toString();
    }
}
