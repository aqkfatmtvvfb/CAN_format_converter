20130415 更新
增加对扩展帧的支持，提供了一个用户输入的选项。

20130224 第一次提交

本小工具用于将特定格式的以逗号分隔的csv文件转为CAN协议数据库dbc格式。
使用方法：
1.参照test_file文件夹下四个例子，填写自定义的报文描述文件；
2.receiver与value description请用空格分隔；
3.所有单元格内容最后不能有空格；
4.byte_order 1 INTEL 2 MOTOROLA
5.value_type + UNSIGNED -SIGNED

