package com.nqc;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.util.Log;
import android.util.Base64;
import android.view.WindowManager;
import android.view.View;
import android.view.Window;
import org.qtproject.qt5.android.bindings.QtActivity;
import org.qtproject.qt5.android.QtNative;

import java.io.OutputStream;
import java.io.InputStream;
import java.util.Scanner;






/**
 * 自定义 Qt Activity，支持 SAF 打开目录并在目录中写入文件
 */
public class FileQtActivity extends QtActivity
{
    private static native void onSafResultNative(String data, String type); //确保Qt端已经实现了该方法
    public static native void onFileSelected(String filePath);
    public static native void onSaveKmlFile(String data, String type);

    private enum OperationType {
        NONE,
        OPEN_DIRECTORY,
        CREATE_FOLDER,
        SAVE_KML,
        OPEN_FILE,
        READ_FILE
    }

    private static final int REQUEST_CODE_OPEN_DIRECTORY = 200;
    private static final int REQUEST_CODE_OPEN_FILE = 201;
    private static final int REQUEST_CODE_SAVE_FILE = 202;//可以让用户修改要保存的文件名

    private static OperationType currentOperation = OperationType.NONE;

    private static Uri selectedFileUri = null;
    private static Uri selectedDirUri = null;
    private static String pendingFolderName = null; // Qt 传来的文件夹名

    private static FileQtActivity instance = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        System.out.println("=== FileQtActivity onCreate called! ===");
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);
               getWindow().getDecorView().setSystemUiVisibility(
                  View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
               );

        instance = this;

    }

    /**
     * 1.1 静态方法，供 Qt 调用：打开 SAF 目录
     */
    public static void openDirectoryFromQt(String folderName,OperationType operationType)
    {
        if(instance == null)  return;
        System.out.println("openDirectoryFromQt!");
        pendingFolderName = folderName;
        currentOperation = operationType;
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE); //打开系统界面让用户选择目录
        instance.startActivityForResult(intent, REQUEST_CODE_OPEN_DIRECTORY); //启动活动并等待结果

    }

    /**
     * 1.2 静态方法，供Qt调用：打开SAF文件选择器
     */
    public static void openFileFromQt()
    {
        if(instance == null)  return;
        System.out.println("openFileFromQt!");
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        instance.startActivityForResult(intent,REQUEST_CODE_OPEN_FILE);

    }


    /**
     * 1.3 静态方法，供Qt调用：打开系统对话框，让用户选择KML保存位置
     */
    public static void openSaveKmlFileDialogFromQt(String defaultName) {
        Activity activity = QtNative.activity();
        if(activity == null) return;
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("application/vnd.google-earth.kml+xml");
        intent.putExtra(Intent.EXTRA_TITLE, defaultName);

        activity.startActivityForResult(intent, REQUEST_CODE_SAVE_FILE);
    }

    /*-----------1系列中的startActivityForResult会触发onActivityResult回调-----------*/


    /**
     * 2 处理 SAF 目录和文件选择回调
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
        if(data == null || resultCode != RESULT_OK) {
            currentOperation = OperationType.NONE;
            return;
        }

        if (requestCode == REQUEST_CODE_OPEN_DIRECTORY)
        {
            selectedDirUri = data.getData();
            // 申请永久权限
            final int takeFlags = data.getFlags() & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            getContentResolver().takePersistableUriPermission(selectedDirUri, takeFlags);

            Log.i("FileQtActivity", "已选择目录: " + selectedDirUri.toString());


            switch(currentOperation)
            {
                case OPEN_DIRECTORY:
                    onSafResultNative(selectedDirUri.toString(), currentOperation.name());
                    break;

                case CREATE_FOLDER:
                    if(pendingFolderName != null) {
                        Uri newFolderUri = createFolderInSelectedDir(pendingFolderName);
                        if(newFolderUri != null) {
                            Log.i("FileQtActivity", "新文件夹 URI: " + newFolderUri.toString());
                        }
                    }
                    break;
            }

        }
        else if(requestCode == REQUEST_CODE_OPEN_FILE)
        {
            selectedFileUri = data.getData();
            final int takeFlags = data.getFlags() & (Intent.FLAG_GRANT_READ_URI_PERMISSION);
            getContentResolver().takePersistableUriPermission(selectedFileUri, takeFlags);

            Log.i("FileQtActivity", "已选择文件: " + selectedFileUri.toString());
            // QtNative.invokeDelegateMethod("onFileSelected",selectedFileUri.toString());
            onFileSelected(selectedFileUri.toString());

            // // 读取文件内容
            // String content = readFileFromSelectedFile();
            // Log.i("FileQtActivity", "文件内容: " + content);

        }
        else if (requestCode == REQUEST_CODE_SAVE_FILE && resultCode == RESULT_OK) {
            Uri uri = data.getData();
            getContentResolver().takePersistableUriPermission( uri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
            );

            onSaveKmlFile(uri.toString(), "SAVE_KML");  // 回调给Qt：类型为 "SAVE_KML"
        }

    }

/*----------------------------------------------------------------------------------------------------------------------*/


    /**
     * 在 SAF 目录创建文件并写入内容content
     */
    public static void saveBinaryFile(String fileName, byte[] data, String mimeType)
    {
        if (instance == null) {
            Log.e("FileQtActivity", "Activity instance is null!");
            return;
        }

        if (selectedDirUri == null) {
            Log.e("FileQtActivity", "No directory selected! createdFolderUri is null");
            return;
        }

        if(currentOperation == OperationType.CREATE_FOLDER) {
            try {
                Uri fileUri = DocumentsContract.createDocument(instance.getContentResolver(), selectedDirUri, mimeType, fileName);
                if (fileUri != null) {
                    try (OutputStream out = instance.getContentResolver().openOutputStream(fileUri)) {
                        if (out != null) {
                            out.write(data);
                            out.flush();
                            // Log.i("FileQtActivity", "文件写入成功: " + fileName);
                        }
                    }
                } else {
                    Log.e("FileQtActivity", "创建文件 URI 失败");
                }
            } catch (Exception e) {
                Log.e("FileQtActivity", "写入文件失败: " + e.getMessage());
            }
        }
        else if(currentOperation == OperationType.OPEN_DIRECTORY) {
            try {
                String treeDocumentId = DocumentsContract.getTreeDocumentId(selectedDirUri);
                Uri documentUri = DocumentsContract.buildDocumentUriUsingTree(selectedDirUri, treeDocumentId);
                Uri fileUri = DocumentsContract.createDocument(instance.getContentResolver(), documentUri, mimeType, fileName);
                if (fileUri != null) {
                    try (OutputStream out = instance.getContentResolver().openOutputStream(fileUri)) {
                        if (out != null) {
                            out.write(data);
                            out.flush();
                        }
                    }
                } else {
                    Log.e("FileQtActivity", "创建文件 URI 失败");
                }
            } catch (Exception e) {
                Log.e("FileQtActivity", "写入文件失败: " + e.getMessage());
            }
        }
    }


    //在 SAF 目录创建文件并写入图片
    private static Uri createFileInSAF(Uri folderUri, String fileName, String mimeType)
    {
        try {
            return DocumentsContract.createDocument(instance.getContentResolver(), folderUri, mimeType, fileName);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static void createImageFileInFolder(String fileName, String base64Content)
    {
        if (selectedDirUri == null) {
            Log.e("FileQtActivity", "Folder not selected");
            return;
        }
        try{
            // 解码 Base64 -> byte[]
             byte[] data = Base64.decode(base64Content, Base64.DEFAULT);

            // 在 SAF 目录下创建文件
            Uri newFileUri = createFileInSAF(selectedDirUri, fileName, "image/png");
            if (newFileUri == null) {
                Log.e("FileQtActivity", "Failed to create image file");
                return;
            }

            // 写入 PNG 数据
            OutputStream outputStream = instance.getContentResolver().openOutputStream(newFileUri);
            outputStream.write(data);
            outputStream.flush();
            outputStream.close();
            Log.i("FileQtActivity", "Image saved: " + newFileUri);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    //向kml文件中写入数据
    public static void saveBytesToUri(String uriStr, byte[] data) {
        Uri uri = Uri.parse(uriStr);
        try {
            OutputStream os = QtNative.activity().getContentResolver().openOutputStream(uri, "w");
            if (os != null) {
                os.write(data);
                os.flush();
                os.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }



    // 在选定目录下创建一个文件夹
    public static Uri createFolderInSelectedDir(String folderName)
    {
        if (instance != null && selectedDirUri != null)
        {
            try {
                String documentId = DocumentsContract.getTreeDocumentId(selectedDirUri);
                Uri documentUri = DocumentsContract.buildDocumentUriUsingTree(selectedDirUri, documentId);

                // 创建文件夹，MIME 类型为 vnd.android.document/directory
                Uri folderUri = DocumentsContract.createDocument(
                    instance.getContentResolver(), documentUri,
                    DocumentsContract.Document.MIME_TYPE_DIR, // 表示文件夹
                    folderName
                );

                if (folderUri != null) {
                    selectedDirUri = folderUri;
                    Log.i("FileQtActivity", "文件夹创建成功: " + folderName);
                    return folderUri;
                } else {
                    Log.e("FileQtActivity", "文件夹创建失败");
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return null;
    }


    /**
    * 读取选定的文件内容
    */
    public static String readFileFromSelectedFile()
    {
        System.out.println("readFileFromSelectedFile!");
        if(instance != null && selectedFileUri != null) {
            try{
                InputStream in = instance.getContentResolver().openInputStream(selectedFileUri);
                if(in != null) {
                    Scanner scanner = new Scanner(in).useDelimiter("\\A");
                    String content = scanner.hasNext() ? scanner.next() : "";
                    in.close();
                    Log.i("FileQtActivity","文件读取成功");
                    return content;
                } else {
                    Log.e("FileQtActivity", "无法打开输入流");
                }
            } catch (Exception e) {
                Log.e("FileQtActivity", "读取文件失败: " + e.getMessage());
                e.printStackTrace();
            }
        } else {
            Log.e("FileQtActivity", "未选择文件或 Activity 为空");
        }
        return "";
    }

    public static String getSelectedFilePath()
    {
        return selectedFileUri.toString();
    }

}
