import scipy.special as sc
import glob
import openpyxl as px

#ブックを新規作成。Workbookの'w'は必ず大文字。
wb = px.Workbook()

#出力ファイル名を指定
filename = "p_value.xlsx"

#アクティブなシートを取得
ws = wb.active

#resultディレクトリ内のファイルをすべてリストに取得
filepath = glob.glob('./chacha/result/*')

#使う値すべて
N = 500                                                                                                        #分割個数
pi = [0.3695, 0.183938, 0.137751, 0.099364, 0.06967, 0.13777]       #πの値(計算済み)
col = 2                                                                                                          #Excel出力に使う値

#list内のファイルを一つずつ開いて処理する
for path in filepath:
    file = open(path, 'r', encoding='utf-8_sig')

    print(file)

    lines = file.readlines()
    file.close()

    #7行ごとに計算するよう
    l = 0

    #結果出力用の準備
    tmp = []
    p_value = []


    #p値を算出してエクセルファイルに出力する
    for value in lines:

        #debug : 値
        print("value=",value)

        if len(value) > 10: #長さが11なのは2進数表示部なので何もしない
            l += 1
        else :
            tmp.append(int(value))  #値部分を取るがstr型なのでint型に変換
            l += 1

            # l == 7つまり、7行tmpリストに入れたらp値を計算する
            if l == 7:
                kai = 0                              #χ^2値
                for vvvv in range(0, 6):    #χ^2計算部分
                    kai += ((tmp[vvvv] - N * pi[vvvv]) ** 2) / (N * pi[vvvv])

                    #debug : kaiの値
                    print("χ=",kai)

                p_value.append((sc.gammainc(2.5, kai / 2))) #p値に変換しlistに保存
                tmp.clear() #tmp listを開放する
                l = 0

    #debug : p値リストの中身
    for ppp in range(1024):
        print(ppp, p_value[ppp])

    #Excel出力部
    for i in range(0, 1024):
        ws.cell(row = i + 1, column = col).value = p_value[i]   #col列目に値を入れていく
    wb.save(filename)

    #1ファイル計算終了後処理
    col += 1                        #Excel出力列を一つずらす
    lines.clear()                  #line listを開放
    p_value.clear()             # p_value listを開放

print("Finish!")
