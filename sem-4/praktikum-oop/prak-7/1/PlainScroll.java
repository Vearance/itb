public class PlainScroll extends Scroll {
    public PlainScroll(String content) {
        super(content);
    }

    @Override
    protected String getType() {
        return "PLAIN";
    }

    @Override
    public String process() {
        String trimmed = getContent().trim().replaceAll("\\s+", " ");
        if (trimmed.isEmpty()) {
            return "";
        }

        String[] words = trimmed.split(" ");
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < words.length; i++) {
            String word = words[i].toLowerCase();
            String capitalized = Character.toUpperCase(word.charAt(0)) + word.substring(1);
            if (i > 0) {
                result.append(" ");
            }
            result.append(capitalized);
        }

        return result.toString();
    }
}
