public abstract class Scroll {
    private final String content;

    public Scroll(String content) {
        this.content = content;
    }

    public String getContent() {
        return content;
    }

    protected abstract String getType();

    public abstract String process();

    @Override
    public String toString() {
        return "[" + getType() + "] " + content;
    }
}
