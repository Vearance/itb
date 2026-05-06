public class APIResponse<U> {
    private int statusCode;
    private String message;
    private U data;

    public APIResponse(int status, String msg, U data) {
        this.statusCode = status;
        this.message = msg;
        this.data = data;
    }

    public void printResponse() {
        String dataStr = (data == null) ? "null" : data.toString();
        String typeName = (data == null) ? "null" : data.getClass().getSimpleName();
        System.out.println("Response " + statusCode + " - " + message + " | Data: " + dataStr + " (Type: " + typeName + ")");
    }
}
