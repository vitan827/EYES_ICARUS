#ifndef INDEX_H
#define INDEX_H

const char* fileManagerHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 File Manager</title>
  <style>
    body { font-family: Arial, sans-serif; max-width: 600px; margin: auto; padding: 20px; }
    summary { cursor: pointer; background: #e0e0e0; padding: 12px; border-radius: 5px; font-weight: bold; list-style: none; }
    summary::-webkit-details-marker { display: none; }
    #fileList { padding: 10px; background: #f9f9f9; border: 1px solid #ddd; margin-top: 5px; border-radius: 5px; }
    li { margin-bottom: 8px; display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid #eee; padding-bottom: 5px;}
    .del-btn { background: #ff4c4c; color: white; border: none; padding: 5px 10px; cursor: pointer; border-radius: 3px; font-size: 12px;}
    .send-btn { background: #008CBA; color: white; border: none; padding: 5px 10px; cursor: pointer; border-radius: 3px; font-size: 12px; margin-right: 5px;}
    .progress-container { width: 100%; background-color: #ddd; border-radius: 4px; margin-top: 15px; display: none; }
    .progress-bar { width: 0%; height: 20px; background-color: #4CAF50; text-align: center; line-height: 20px; color: white; border-radius: 4px; font-size: 12px; }
  </style>
</head>
<body>
  <h2>He Thong Quan Ly File (.txt)</h2>
  <form method='POST' action='/upload' enctype='multipart/form-data'>
    <input type='file' name='file' accept='.txt'><br><br>
    <input type='submit' value='Tai len'>
  </form>

  <div style="margin: 20px 0; padding: 15px; background: #f4f4f4; border-radius: 5px;">
    <h3>Điều khiển Arduino Nano</h3>
    <button onclick="sendToNano()" style="padding: 10px 15px; background: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer;">Kích hoạt chuyển dữ liệu</button>
  </div>

  <div id="transferStatus" style="font-weight: bold; margin-top: 10px; color: #333;"></div>
  <div class="progress-container" id="pContainer"><div class="progress-bar" id="pBar">0%</div></div>

  <hr>
  
  <details open>
    <summary>📂 Xem danh sach file hien co</summary>
    <div id="fileList">Dang tai du lieu...</div>
  </details>

  <script>
    const maxFiles = 10;
    function loadFiles() {
      fetch('/list').then(res => res.json()).then(data => {
        let listDiv = document.getElementById('fileList');
        if(data.length === 0) {
          listDiv.innerHTML = "Khong co file nao trong bo nho.";
          return;
        }
        
        let html = "<ul style='padding-left: 0;'>";
        let limit = Math.min(data.length, maxFiles);
        for(let i = 0; i < limit; i++) {
          html += `<li>
            <span><a href="/download?file=${data[i].name}">${data[i].name}</a> (${data[i].size} bytes)</span>
            <div>
              <button class='send-btn' onclick="sendFile('${data[i].name}')">Truyền</button>
              <button class='del-btn' onclick="deleteFile('${data[i].name}')">Xoa</button>
            </div>
          </li>`;
        }
        html += "</ul>";
        
        if(data.length > maxFiles) {
          html += `<p><i>* Đang hiển thị ${maxFiles}/${data.length} file.</i></p>`;
        }
        listDiv.innerHTML = html;
      }).catch(err => {
        document.getElementById('fileList').innerHTML = "Loi khi tai danh sach file.";
      });
    }

    function deleteFile(filename) {
      if(confirm('Ban co chac muon xoa file: ' + filename + '?')) {
        fetch('/delete?file=' + filename, { method: 'DELETE' })
          .then(res => {
            if(res.ok) {
              loadFiles();
            } else {
              alert('Xoa that bai!');
            }
          });
      }
    }

    loadFiles();

    function sendToNano() {
      fetch('/send-nano', { method: 'POST' })
        .then(res => {
          if(res.ok) alert('Đã gửi lệnh sang Arduino Nano thành công!');
          else alert('Gửi lệnh thất bại!');
        });
    }

    async function sendFile(filename) {
      const statusText = document.getElementById('transferStatus');
      const pContainer = document.getElementById('pContainer');

      statusText.innerText = "Đang xử lý truyền file: " + filename + " trực tiếp từ ESP32...";
      pContainer.style.display = 'none'; 

      try {
        let response = await fetch(`/transfer-direct?file=${filename}`, {
          method: 'POST'
        });

        if (!response.ok) throw new Error("ESP32 từ chối yêu cầu hoặc có lỗi xảy ra.");
        
        statusText.innerText = "Hoàn tất lệnh truyền file: " + filename;
      } catch (error) {
        statusText.innerText = "❌ Lỗi: " + error.message;
      }
    }

  </script>
</body>
</html>
)rawliteral";

#endif