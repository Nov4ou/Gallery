package com.example.galleryuploader;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends Activity {
    private static final int PICK_IMAGE_REQUEST = 1001;
    private static final int CONNECT_TIMEOUT_MS = 6000;
    private static final int RAW_CHUNK_SIZE = 512;
    private static final int CONNECT_SETTLE_DELAY_MS = 1200;
    private static final int SEND_DELAY_MS = 500;
    private static final int TARGET_IMAGE_MAX_WIDTH = 480;
    private static final int TARGET_IMAGE_MAX_HEIGHT = 800;
    private static final int JPEG_QUALITY = 88;

    private final ExecutorService executor = Executors.newSingleThreadExecutor();

    private EditText hostEdit;
    private EditText portEdit;
    private EditText placeEdit;
    private EditText fileNameEdit;
    private Spinner categorySpinner;
    private Button pickButton;
    private Button sendButton;
    private ImageView previewImage;
    private ProgressBar progressBar;
    private TextView statusText;

    private File selectedCacheFile;
    private Uri selectedUri;
    private String selectedDisplayName = "";

    private final CategoryItem[] categories = {
            new CategoryItem("Scenery (SCN)", "SCN"),
            new CategoryItem("Building (BLD)", "BLD"),
            new CategoryItem("Plant (PLT)", "PLT"),
            new CategoryItem("Person (PER)", "PER"),
            new CategoryItem("Animal (ANI)", "ANI")
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        hostEdit = findViewById(R.id.hostEdit);
        portEdit = findViewById(R.id.portEdit);
        placeEdit = findViewById(R.id.placeEdit);
        fileNameEdit = findViewById(R.id.fileNameEdit);
        categorySpinner = findViewById(R.id.categorySpinner);
        pickButton = findViewById(R.id.pickButton);
        sendButton = findViewById(R.id.sendButton);
        previewImage = findViewById(R.id.previewImage);
        progressBar = findViewById(R.id.progressBar);
        statusText = findViewById(R.id.statusText);

        ArrayAdapter<CategoryItem> adapter = new ArrayAdapter<>(
                this, android.R.layout.simple_spinner_item, categories);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        categorySpinner.setAdapter(adapter);

        pickButton.setOnClickListener(v -> pickImage());
        sendButton.setOnClickListener(v -> sendSelectedImage());

        categorySpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                refreshSuggestedFileName();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        placeEdit.addTextChangedListener(new SimpleTextWatcher() {
            @Override
            public void afterTextChanged(Editable s) {
                refreshSuggestedFileName();
            }
        });

        refreshSuggestedFileName();
    }

    @Override
    protected void onDestroy() {
        executor.shutdownNow();
        super.onDestroy();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode != PICK_IMAGE_REQUEST || resultCode != RESULT_OK || data == null) {
            return;
        }

        Uri uri = data.getData();
        if (uri == null) {
            setStatus("No image selected.");
            return;
        }

        selectedUri = uri;
        selectedDisplayName = queryDisplayName(uri);
        previewImage.setImageURI(uri);
        sendButton.setEnabled(false);
        progressBar.setProgress(0);
        setStatus("Resizing image...");

        executor.execute(() -> {
            try {
                File cacheFile = resizeUriToJpegCache(uri);
                selectedCacheFile = cacheFile;
                runOnUiThread(() -> {
                    previewImage.setImageURI(Uri.fromFile(cacheFile));
                    sendButton.setEnabled(true);
                    setStatus("Selected: " + selectedDisplayName +
                            "\nResized to fit " + TARGET_IMAGE_MAX_WIDTH + "x" + TARGET_IMAGE_MAX_HEIGHT +
                            "\nSend size: " + cacheFile.length() + " bytes");
                });
            } catch (IOException e) {
                runOnUiThread(() -> setStatus("Failed to resize image: " + e.getMessage()));
            }
        });
    }

    private void pickImage() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("image/*");
        startActivityForResult(intent, PICK_IMAGE_REQUEST);
    }

    private void sendSelectedImage() {
        File file = selectedCacheFile;
        if (file == null || !file.exists()) {
            Toast.makeText(this, "Pick a photo first.", Toast.LENGTH_SHORT).show();
            return;
        }

        String host = hostEdit.getText().toString().trim();
        int port;
        try {
            port = Integer.parseInt(portEdit.getText().toString().trim());
        } catch (NumberFormatException e) {
            Toast.makeText(this, "Invalid port.", Toast.LENGTH_SHORT).show();
            return;
        }

        String fileName = sanitizeFileName(fileNameEdit.getText().toString().trim());
        if (fileName.isEmpty()) {
            Toast.makeText(this, "File name is empty.", Toast.LENGTH_SHORT).show();
            return;
        }
        if (!fileName.toLowerCase(Locale.US).endsWith(".jpg") &&
                !fileName.toLowerCase(Locale.US).endsWith(".jpeg")) {
            fileName = fileName + ".jpg";
        }

        sendButton.setEnabled(false);
        progressBar.setProgress(0);
        setStatus("Connecting to " + host + ":" + port + "...");

        String finalFileName = fileName;
        executor.execute(() -> {
            try {
                uploadFile(host, port, finalFileName, file);
                runOnUiThread(() -> {
                    progressBar.setProgress(100);
                    sendButton.setEnabled(true);
                    setStatus("Upload complete.\nFile: " + finalFileName);
                });
            } catch (IOException e) {
                runOnUiThread(() -> {
                    sendButton.setEnabled(true);
                    setStatus("Upload failed: " + e.getMessage());
                });
            }
        });
    }

    private void uploadFile(String host, int port, String fileName, File file) throws IOException {
        long totalBytes = file.length();
        if (totalBytes <= 0) {
            throw new IOException("selected image is empty");
        }

        try (Socket socket = createWifiSocket()) {
            socket.connect(new InetSocketAddress(host, port), CONNECT_TIMEOUT_MS);
            sleepMs(CONNECT_SETTLE_DELAY_MS);
            OutputStream socketOutput = socket.getOutputStream();

            String header = "IMGHEX:" + totalBytes + ":" + fileName + "\n";
            socketOutput.write(header.getBytes(StandardCharsets.US_ASCII));
            socketOutput.flush();
            sleepMs(SEND_DELAY_MS);
            runOnUiThread(() -> setStatus("Header sent.\nSending image..."));

            byte[] raw = new byte[RAW_CHUNK_SIZE];
            byte[] hex = new byte[RAW_CHUNK_SIZE * 2];
            long sentRawBytes = 0;

            try (BufferedInputStream fileInput = new BufferedInputStream(new FileInputStream(file))) {
                int read;
                while ((read = fileInput.read(raw)) > 0) {
                    int hexLen = encodeHex(raw, read, hex);
                    socketOutput.write(hex, 0, hexLen);
                    socketOutput.flush();
                    sleepMs(SEND_DELAY_MS);

                    sentRawBytes += read;
                    int progress = (int) ((sentRawBytes * 100L) / totalBytes);
                    runOnUiThread(() -> {
                        progressBar.setProgress(progress);
                        setStatus("Sending image...\n" + progress + "%");
                    });
                }
            }
        }
    }

    private Socket createWifiSocket() throws IOException {
        ConnectivityManager connectivityManager =
                (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);

        if (connectivityManager != null) {
            Network[] networks = connectivityManager.getAllNetworks();
            for (Network network : networks) {
                NetworkCapabilities capabilities =
                        connectivityManager.getNetworkCapabilities(network);
                if (capabilities != null &&
                        capabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
                    return network.getSocketFactory().createSocket();
                }
            }
        }

        return new Socket();
    }

    private void sleepMs(int delayMs) throws IOException {
        try {
            Thread.sleep(delayMs);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new IOException("upload interrupted", e);
        }
    }

    private int encodeHex(byte[] raw, int rawLen, byte[] hexOut) {
        final byte[] digits = "0123456789ABCDEF".getBytes(StandardCharsets.US_ASCII);
        int out = 0;

        for (int i = 0; i < rawLen; i++) {
            int value = raw[i] & 0xFF;
            hexOut[out++] = digits[value >> 4];
            hexOut[out++] = digits[value & 0x0F];
        }

        return out;
    }

    private File resizeUriToJpegCache(Uri uri) throws IOException {
        BitmapFactory.Options bounds = new BitmapFactory.Options();
        bounds.inJustDecodeBounds = true;

        try (InputStream input = getContentResolver().openInputStream(uri)) {
            if (input == null) {
                throw new IOException("cannot open selected image");
            }
            BitmapFactory.decodeStream(input, null, bounds);
        }

        if (bounds.outWidth <= 0 || bounds.outHeight <= 0) {
            throw new IOException("unsupported image format");
        }

        BitmapFactory.Options decodeOptions = new BitmapFactory.Options();
        decodeOptions.inSampleSize = calculateInSampleSize(
                bounds.outWidth,
                bounds.outHeight,
                TARGET_IMAGE_MAX_WIDTH,
                TARGET_IMAGE_MAX_HEIGHT);

        Bitmap decoded;
        try (InputStream input = getContentResolver().openInputStream(uri)) {
            if (input == null) {
                throw new IOException("cannot open selected image");
            }
            decoded = BitmapFactory.decodeStream(input, null, decodeOptions);
        }

        if (decoded == null) {
            throw new IOException("cannot decode selected image");
        }

        int outWidth = decoded.getWidth();
        int outHeight = decoded.getHeight();
        float scale = Math.min(
                TARGET_IMAGE_MAX_WIDTH / (float) outWidth,
                TARGET_IMAGE_MAX_HEIGHT / (float) outHeight);

        Bitmap outputBitmap = decoded;
        if (scale < 1.0f) {
            int scaledWidth = Math.max(1, Math.round(outWidth * scale));
            int scaledHeight = Math.max(1, Math.round(outHeight * scale));
            outputBitmap = Bitmap.createScaledBitmap(decoded, scaledWidth, scaledHeight, true);
        }

        File file = File.createTempFile("gallery_upload_", ".jpg", getCacheDir());
        try (FileOutputStream output = new FileOutputStream(file)) {
            if (!outputBitmap.compress(Bitmap.CompressFormat.JPEG, JPEG_QUALITY, output)) {
                throw new IOException("cannot encode resized image");
            }
        } finally {
            if (outputBitmap != decoded) {
                outputBitmap.recycle();
            }
            decoded.recycle();
        }

        return file;
    }

    private int calculateInSampleSize(int srcWidth, int srcHeight, int reqWidth, int reqHeight) {
        int inSampleSize = 1;

        while ((srcWidth / (inSampleSize * 2)) >= reqWidth &&
                (srcHeight / (inSampleSize * 2)) >= reqHeight) {
            inSampleSize *= 2;
        }

        return inSampleSize;
    }

    private String queryDisplayName(Uri uri) {
        try (Cursor cursor = getContentResolver().query(uri, null, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                int index = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (index >= 0) {
                    String name = cursor.getString(index);
                    if (name != null && !name.isEmpty()) {
                        return name;
                    }
                }
            }
        } catch (Exception ignored) {
        }

        return "selected.jpg";
    }

    private void refreshSuggestedFileName() {
        CategoryItem item = (CategoryItem) categorySpinner.getSelectedItem();
        String category = item == null ? "SCN" : item.code;
        String place = sanitizeToken(placeEdit == null ? "Phone" : placeEdit.getText().toString());
        if (place.isEmpty()) {
            place = "Phone";
        }

        String timestamp = new SimpleDateFormat("yyyyMMdd_HHmm", Locale.US).format(new Date());
        fileNameEdit.setText(category + "_" + timestamp + "_" + place + ".jpg");
    }

    private String sanitizeToken(String text) {
        if (text == null) {
            return "";
        }

        StringBuilder out = new StringBuilder();
        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9')) {
                out.append(c);
            } else if (c == '_' || c == '-' || c == ' ') {
                out.append('_');
            }
        }

        return out.toString().replaceAll("_+", "_").replaceAll("^_|_$", "");
    }

    private String sanitizeFileName(String text) {
        if (text == null) {
            return "";
        }

        String base = text.trim();
        String lower = base.toLowerCase(Locale.US);
        String extension = ".jpg";

        if (lower.endsWith(".jpeg")) {
            base = base.substring(0, base.length() - 5);
            extension = ".jpeg";
        } else if (lower.endsWith(".jpg")) {
            base = base.substring(0, base.length() - 4);
        }

        String token = sanitizeToken(base);
        if (token.isEmpty()) {
            return "";
        }

        return token + extension;
    }

    private void setStatus(String text) {
        statusText.setText(text);
    }

    private static class CategoryItem {
        final String label;
        final String code;

        CategoryItem(String label, String code) {
            this.label = label;
            this.code = code;
        }

        @Override
        public String toString() {
            return label;
        }
    }

    private abstract static class SimpleTextWatcher implements TextWatcher {
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
        }
    }
}
