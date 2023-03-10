import numpy as np

class MaxPooling:
    '''
    Max Pooling of input
    '''
    def __init__(self, kernel_size, stride):
        self.kernel_size = kernel_size
        self.stride = stride
        self.cache = None
        self.dx = None

    def forward(self, x):
        '''
        Forward pass of max pooling
        :param x: input, (N, C, H, W)
        :return: The output by max pooling with kernel_size and stride
        '''
        out = None
        #############################################################################
        # TODO: Implement the max pooling forward pass.                             #
        # Hint:                                                                     #
        #       1) You may implement the process with loops                         #
        #############################################################################
        as_strided = np.lib.stride_tricks.as_strided
        N, C, H, W = x.shape
        NS, CS, HS, WS = x.strides
        H_out = (H - self.kernel_size) // self.stride + 1
        W_out = (W - self.kernel_size) // self.stride + 1

        window_shape = (N, C, H_out, W_out, self.kernel_size, self.kernel_size)
        window_strides = (NS, CS, HS * self.stride, WS * self.stride, HS, WS)

        x_view = as_strided(x, window_shape, window_strides)
        out = x_view.max(axis=(4, 5))

        #############################################################################
        #                              END OF YOUR CODE                             #
        #############################################################################
        self.cache = (x, H_out, W_out)
        return out

    def backward(self, dout):
        '''
        Backward pass of max pooling
        :param dout: Upstream derivatives
        :return:
        '''
        x, H_out, W_out = self.cache
        #############################################################################
        # TODO: Implement the max pooling backward pass.                            #
        # Hint:                                                                     #
        #       1) You may implement the process with loops                     #
        #       2) You may find np.unravel_index useful                             #
        #############################################################################
        dx = np.zeros_like(x)
        N, C, H, W = x.shape
        k = self.kernel_size
        for n in range(N):
            for c in range(C):
                curr_y = out_y = 0
                while curr_y + k <= H:
                    curr_x = out_x = 0
                    while curr_x + k <= W:
                        pool = x[n, c, curr_y : curr_y + k, curr_x: curr_x + k]
                        i, j = np.unravel_index(np.argmax(pool), pool.shape)
                        dx[n, c, curr_y + i, curr_x + j] = dout[n, c, out_y, out_x]
                        curr_x += self.stride
                        out_x += 1
                    curr_y += self.stride
                    out_y += 1
            
        self.dx = dx
        #############################################################################
        #                              END OF YOUR CODE                             #
        #############################################################################
